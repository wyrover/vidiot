#include "GuiTimeLine.h"
#include <boost/make_shared.hpp>
#include <boost/foreach.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include "wxInclude.h"
#include <wx/dcclient.h>
#include <wx/pen.h>
#include <math.h>
#include <algorithm>
#include "UtilLog.h"
#include "GuiMain.h"
#include "GuiOptions.h"
#include "GuiPreview.h"
#include "GuiWindow.h"
#include "GuiTimeLineDropTarget.h"
#include "GuiTimeLineZoom.h"
#include "GuiTimeLineClip.h"
#include "GuiTimeLineTrack.h"
#include "AProjectViewNode.h"
#include "Sequence.h"
#include "VideoTrack.h"
#include "AudioTrack.h"
#include "ProjectEventAddAsset.h"
#include "ProjectEventDeleteAsset.h"
#include "ProjectEventRenameAsset.h"

static int sTimeScaleMinutesHeight = 10;
static int sTimeScaleSecondHeight = 5;
static int sTimeScaleHeight = 20;
static int sMinimalGreyAboveVideoTracksHeight = 10;
static int sMinimalGreyBelowAudioTracksHeight = 10;

static int sDefaultAudioVideoDividerPosition = 100;
static int sAudioVideoDividerHeight = 5;

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

GuiTimeLine::GuiTimeLine(model::SequencePtr sequence)
:   wxScrolledWindow()
,   mZoom(boost::make_shared<GuiTimeLineZoom>()) // May be reset upon recovery
,   mCursorPosition(0)
,   mPlaybackTime(0)
,   mOrigin(0,100)
,   mBitmap()
,   mMouseState(*this)
,   mWidth(0)
,   mHeight(0)
,   mPlayer()
,   mDividerPosition(0)
,   mSequence(sequence)
,   m_dragImage(0)
{
	LOG_INFO;

    if (mSequence)
    {
        // mSequence is initialized when creating a new timeline.
        // When recovering, the tracks are recreated from the file.
        BOOST_FOREACH( model::VideoTrackPtr track, mSequence->getVideoTracks())
        {
            mVideoTracks.push_back(boost::make_shared<GuiTimeLineTrack>(mZoom, boost::static_pointer_cast<model::Track>(track)));
        }
        BOOST_FOREACH( model::AudioTrackPtr track, mSequence->getAudioTracks())
        {
            mAudioTracks.push_back(boost::make_shared<GuiTimeLineTrack>(mZoom, boost::static_pointer_cast<model::Track>(track)));
        }
    }
}

void GuiTimeLine::init(wxWindow *parent)
{
    Create(parent,wxID_ANY,wxPoint(0,0),wxDefaultSize,wxHSCROLL|wxVSCROLL|wxSUNKEN_BORDER);

    SetScrollRate( 10, 10 );
    EnableScrolling(true,true);
    SetBackgroundColour(* wxLIGHT_GREY);
    SetDropTarget(new GuiTimeLineDropTarget(mZoom,this));

    // Must be done before initializing tracks, since tracks derive their width from the entire timeline
    DetermineWidth();

    // Initialize tracks (this also creates the bitmaps)
    BOOST_REVERSE_FOREACH( GuiTimeLineTrackPtr track, mVideoTracks )
    {
        track->init(this);
    }
    BOOST_FOREACH( GuiTimeLineTrackPtr track, mAudioTracks )
    {
        track->init(this);
    }

    wxGetApp().Bind(PROJECT_EVENT_ADD_ASSET,        &GuiTimeLine::OnProjectAssetAdded,     this);
    wxGetApp().Bind(PROJECT_EVENT_DELETE_ASSET,     &GuiTimeLine::OnProjectAssetDeleted,   this);
    wxGetApp().Bind(PROJECT_EVENT_RENAME_ASSET,     &GuiTimeLine::OnProjectAssetRenamed,   this);

    Bind(wxEVT_PAINT,               &GuiTimeLine::OnPaint,              this);
    Bind(wxEVT_ERASE_BACKGROUND,    &GuiTimeLine::OnEraseBackground,    this);
    Bind(wxEVT_SIZE,                &GuiTimeLine::OnSize,               this);
    Bind(wxEVT_MOTION,              &GuiTimeLine::OnMouseEvent,         this);
    Bind(wxEVT_LEFT_DOWN,           &GuiTimeLine::OnMouseEvent,         this);
    Bind(wxEVT_LEFT_UP,             &GuiTimeLine::OnMouseEvent,         this);
    Bind(wxEVT_LEFT_DCLICK,         &GuiTimeLine::OnMouseEvent,         this);
    Bind(wxEVT_MIDDLE_DOWN,         &GuiTimeLine::OnMouseEvent,         this);
    Bind(wxEVT_MIDDLE_UP,           &GuiTimeLine::OnMouseEvent,         this);
    Bind(wxEVT_MIDDLE_DCLICK,       &GuiTimeLine::OnMouseEvent,         this);
    Bind(wxEVT_RIGHT_DOWN,          &GuiTimeLine::OnMouseEvent,         this);
    Bind(wxEVT_RIGHT_UP,            &GuiTimeLine::OnMouseEvent,         this);
    Bind(wxEVT_RIGHT_DCLICK,        &GuiTimeLine::OnMouseEvent,         this);
    Bind(wxEVT_ENTER_WINDOW,        &GuiTimeLine::OnMouseEvent,         this);
    Bind(wxEVT_LEAVE_WINDOW,        &GuiTimeLine::OnMouseEvent,         this);
    Bind(wxEVT_MOUSEWHEEL,          &GuiTimeLine::OnMouseEvent,         this);
    Bind(wxEVT_KEY_DOWN,            &GuiTimeLine::OnKeyDown,            this);
    Bind(wxEVT_KEY_UP,              &GuiTimeLine::OnKeyUp,              this);

    BOOST_FOREACH( GuiTimeLineTrackPtr track, mVideoTracks )
    {
        track->Bind(TRACK_UPDATE_EVENT,    &GuiTimeLine::OnTrackUpdated,       this);
    }
    BOOST_FOREACH( GuiTimeLineTrackPtr track, mAudioTracks )
    {
        track->Bind(TRACK_UPDATE_EVENT,    &GuiTimeLine::OnTrackUpdated,       this);
    }

    ASSERT(mSequence);
    mPlayer = dynamic_cast<GuiWindow*>(wxGetApp().GetTopWindow())->getPreview().openTimeline(this);

    // From here on, processing continues with size events after laying out this widget.
}

GuiTimeLine::~GuiTimeLine()
{
    if (m_dragImage)
        delete m_dragImage;

    wxGetApp().Unbind(PROJECT_EVENT_ADD_ASSET,      &GuiTimeLine::OnProjectAssetAdded,      this);
    wxGetApp().Unbind(PROJECT_EVENT_DELETE_ASSET,   &GuiTimeLine::OnProjectAssetDeleted,    this);
    wxGetApp().Unbind(PROJECT_EVENT_RENAME_ASSET,   &GuiTimeLine::OnProjectAssetRenamed,    this);

    dynamic_cast<GuiWindow*>(wxGetApp().GetTopWindow())->getPreview().closeTimeline(this);

    Unbind(wxEVT_PAINT,               &GuiTimeLine::OnPaint,              this);
    Unbind(wxEVT_ERASE_BACKGROUND,    &GuiTimeLine::OnEraseBackground,    this);
    Unbind(wxEVT_SIZE,                &GuiTimeLine::OnSize,               this);
    Unbind(wxEVT_MOTION,              &GuiTimeLine::OnMouseEvent,         this);
    Unbind(wxEVT_LEFT_DOWN,           &GuiTimeLine::OnMouseEvent,         this);
    Unbind(wxEVT_LEFT_UP,             &GuiTimeLine::OnMouseEvent,         this);
    Unbind(wxEVT_LEFT_DCLICK,         &GuiTimeLine::OnMouseEvent,         this);
    Unbind(wxEVT_MIDDLE_DOWN,         &GuiTimeLine::OnMouseEvent,         this);
    Unbind(wxEVT_MIDDLE_UP,           &GuiTimeLine::OnMouseEvent,         this);
    Unbind(wxEVT_MIDDLE_DCLICK,       &GuiTimeLine::OnMouseEvent,         this);
    Unbind(wxEVT_RIGHT_DOWN,          &GuiTimeLine::OnMouseEvent,         this);
    Unbind(wxEVT_RIGHT_UP,            &GuiTimeLine::OnMouseEvent,         this);
    Unbind(wxEVT_RIGHT_DCLICK,        &GuiTimeLine::OnMouseEvent,         this);
    Unbind(wxEVT_ENTER_WINDOW,        &GuiTimeLine::OnMouseEvent,         this);
    Unbind(wxEVT_LEAVE_WINDOW,        &GuiTimeLine::OnMouseEvent,         this);
    Unbind(wxEVT_MOUSEWHEEL,          &GuiTimeLine::OnMouseEvent,         this);
    Unbind(wxEVT_KEY_DOWN,            &GuiTimeLine::OnKeyDown,            this);
    Unbind(wxEVT_KEY_UP,              &GuiTimeLine::OnKeyUp,              this);

    BOOST_FOREACH( GuiTimeLineTrackPtr track, mVideoTracks )
    {
        track->Unbind(TRACK_UPDATE_EVENT,    &GuiTimeLine::OnTrackUpdated,       this);
    }
    BOOST_FOREACH( GuiTimeLineTrackPtr track, mAudioTracks )
    {
        track->Unbind(TRACK_UPDATE_EVENT,    &GuiTimeLine::OnTrackUpdated,       this);
    }
}

//////////////////////////////////////////////////////////////////////////
// MODEL EVENTS
//////////////////////////////////////////////////////////////////////////

void GuiTimeLine::OnProjectAssetAdded( ProjectEventAddAsset &event )
{
    VAR_DEBUG(event.getParent())(event.getNode());
    if (event.getParent() == mSequence)
    {
        NIY;            // todo make adding the track with the right position
        Refresh();
    }
    event.Skip();
}

void GuiTimeLine::OnProjectAssetDeleted( ProjectEventDeleteAsset &event )
{
    VAR_DEBUG(event.getParent())(event.getNode());
    if (event.getParent() == mSequence)
    {
        NIY; // todo reposition tracks if needed
        Refresh();
    }
    event.Skip();
}

void GuiTimeLine::OnProjectAssetRenamed( ProjectEventRenameAsset &event )
{
    VAR_DEBUG(event.getNode());
    if (event.getNode() == mSequence)
    {
        Refresh();
    }
    event.Skip();
}

//////////////////////////////////////////////////////////////////////////
// GUI EVENTS
//////////////////////////////////////////////////////////////////////////

void GuiTimeLine::OnKeyDown(wxKeyEvent& event)
{

}
void GuiTimeLine::OnKeyUp(wxKeyEvent& event)
{

}

void GuiTimeLine::OnMouseEvent(wxMouseEvent& event)
{
    GuiTimeLineClipPtr clip = findClip(event.GetPosition());

    wxMemoryDC dc(mBitmap);
    DoPrepareDC(dc);
    wxPoint virtualposition = event.GetLogicalPosition(dc);

    mMouseState.processMouseEvent(virtualposition, clip, event);
}

void GuiTimeLine::beginDrag(wxPoint position)
{
    VAR_INFO(position);
    m_dragImage = new GuiTimeLineDragImage(this, position);

    bool ok = m_dragImage->BeginDrag(m_dragImage->getHotspot(), this, false);
    ASSERT(ok);

    moveDrag(position);
}

void GuiTimeLine::moveDrag(wxPoint position)
{
    VAR_INFO(position);
    m_dragImage->Hide();
    Refresh(false);
    Update();
    m_dragImage->Move(position);
    m_dragImage->Show();
}

void GuiTimeLine::endDrag(wxPoint position)
{
    VAR_INFO(position);
    m_dragImage->Hide();
    m_dragImage->EndDrag();
    Refresh();
    delete m_dragImage;
    m_dragImage = 0;
}

void GuiTimeLine::OnSize(wxSizeEvent& event) 
{
    DetermineHeight();

    mDividerPosition = 
        sTimeScaleHeight + 
        sMinimalGreyAboveVideoTracksHeight + 
        (mHeight - sTimeScaleHeight - sMinimalGreyAboveVideoTracksHeight - sAudioVideoDividerHeight) / 2;

    updateSize(); // Triggers the initial drawing
}


void GuiTimeLine::OnEraseBackground(wxEraseEvent& event)
{
    //event.Skip(); // The official way of doing it
}

void GuiTimeLine::OnPaint( wxPaintEvent &WXUNUSED(event) )
{
    wxPaintDC dc( this );
    DoPrepareDC(dc); // Adjust for logical coordinates, not device coordinates

    // Find Out where the window is scrolled to
    int scrollX, scrollY, ppuX, ppuY;
    GetViewStart(&scrollX,&scrollY);
    GetScrollPixelsPerUnit(&ppuX,&ppuY);
    int virtualbaseX = scrollX * ppuX;
    int virtualbaseY = scrollY * ppuY;

    wxMemoryDC dcBmp(mBitmap);

    wxRegionIterator upd(GetUpdateRegion()); // get the update rect list
    while (upd)
    {
        int x = virtualbaseX + upd.GetX();
        int y = virtualbaseY + upd.GetY();
        int w = upd.GetW();
        int h = upd.GetH();
        dc.Blit(x,y,w,h,&dcBmp,x,y,wxCOPY,false,0,0);
        upd++;
    }

    // Draw cursor
    dc.SetPen(wxPen(*wxRED, 1));
    dc.DrawLine(wxPoint(mCursorPosition,0),wxPoint(mCursorPosition,mHeight));
}

//////////////////////////////////////////////////////////////////////////
// DRAWING EVENTS
//////////////////////////////////////////////////////////////////////////

void GuiTimeLine::OnTrackUpdated( TrackUpdateEvent& event )
{
    LOG_INFO;
    /** todo only redraw track */
    updateBitmap();
    Update();
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

model::SequencePtr GuiTimeLine::getSequence() const
{ 
    return mSequence;
}

int GuiTimeLine::getWidth() const
{
    return mWidth;
}

//////////////////////////////////////////////////////////////////////////
// CURSOR
//////////////////////////////////////////////////////////////////////////

void GuiTimeLine::setCursorPosition(long position)
{
    VAR_DEBUG(mCursorPosition)(position);

    long oldPos = mCursorPosition;
    mCursorPosition = position;

    RefreshRect(wxRect(std::min(mCursorPosition,oldPos),0,std::abs(mCursorPosition-oldPos)+1,mHeight),false);
}

void GuiTimeLine::moveCursorOnPlayback(long pts)
{
    setCursorPosition(mZoom->ptsToPixels(pts));
}

void GuiTimeLine::moveCursorOnUser(int position)
{
    setCursorPosition(position);
    mPlayer->moveTo(mZoom->pixelsToPts(position));
}

//////////////////////////////////////////////////////////////////////////
// FROM COORDINATES TO OBJECTS
//////////////////////////////////////////////////////////////////////////


GuiTimeLineClipPtr GuiTimeLine::findClip(wxPoint p) const
{
    GuiTimeLineTrackPtr track = findTrack(p.y);
    if (track)
    {
        return track->findClip(p.x);
    }
    return GuiTimeLineClipPtr();
}

GuiTimeLineTrackPtr GuiTimeLine::findTrack(int yposition) const
{
    int top = mDividerPosition;
    BOOST_REVERSE_FOREACH( GuiTimeLineTrackPtr track, mVideoTracks )
    {
        int bottom = top;
        top -= track->getBitmap().GetHeight();
        if (yposition <= bottom && yposition >= top) return track;
    }
    int bottom = mDividerPosition + sAudioVideoDividerHeight;
    BOOST_FOREACH( GuiTimeLineTrackPtr track, mAudioTracks )
    {
        int top = bottom;
        bottom += track->getBitmap().GetHeight();
        if (yposition <= bottom && yposition >= top) return track;
    }
    return GuiTimeLineTrackPtr();
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void GuiTimeLine::DetermineWidth()
{
    mWidth = std::max(std::max(
        mZoom->timeToPixels(5 * Constants::sMinute),            // Minimum width of 5 minutes
        mZoom->ptsToPixels(mSequence->getNumberOfFrames())),    // At least enough to hold all clips
        GetClientSize().GetWidth());                            // At least the widget size
}

void GuiTimeLine::DetermineHeight()
{
    int requiredHeight = sTimeScaleHeight;
    requiredHeight += sMinimalGreyAboveVideoTracksHeight;
    BOOST_REVERSE_FOREACH( GuiTimeLineTrackPtr track, mVideoTracks )
    {
        requiredHeight += track->getBitmap().GetHeight();
    }
    requiredHeight += sAudioVideoDividerHeight;
    BOOST_FOREACH( GuiTimeLineTrackPtr track, mAudioTracks )
    {
        requiredHeight += track->getBitmap().GetHeight();
    }
    requiredHeight += sMinimalGreyBelowAudioTracksHeight;

    mHeight = std::max(requiredHeight, GetClientSize().GetHeight());
}

void GuiTimeLine::updateSize()
{
    DetermineWidth();
    DetermineHeight();

    SetVirtualSize(mWidth,mHeight);
    mBitmap.Create(mWidth,mHeight);

    updateBitmap();
}

 void GuiTimeLine::updateBitmap()
 {
     LOG_INFO;
     // todo move to update background
     wxMemoryDC dc(mBitmap);

     // Get size of canvas
     int w = mBitmap.GetWidth();
     int h = mBitmap.GetHeight();

     // Set BG
     dc.SetPen(*wxLIGHT_GREY_PEN);
     dc.SetBrush(*wxLIGHT_GREY_BRUSH);
     dc.DrawRectangle(0,0,w,h);

     // Draw timescale
     dc.SetBrush(wxNullBrush);
     wxPen blackLinePen(*wxBLACK, 1);
     dc.SetPen(blackLinePen);
     dc.DrawRectangle(0,0,w,sTimeScaleHeight);

     wxFont* f = const_cast<wxFont*>(wxSMALL_FONT);
     dc.SetFont(*f);

     // Draw seconds and minutes lines
     for (int ms = 0; mZoom->timeToPixels(ms) <= w; ms += Constants::sSecond)
     {
         int position = mZoom->timeToPixels(ms);
         bool isMinute = (ms % Constants::sMinute == 0);
         int height = sTimeScaleSecondHeight;

         if (isMinute)
         {
             height = sTimeScaleMinutesHeight;
         }

         dc.DrawLine(position,0,position,height);

         if (ms == 0)
         {
             dc.DrawText( "0", 5, sTimeScaleMinutesHeight );
         }
         else
         {
             if (isMinute)
             {
                 wxDateTime t(ms / Constants::sHour, (ms % Constants::sHour) / Constants::sMinute, (ms % Constants::sMinute) / Constants::sSecond, ms % Constants::sSecond);
                 wxString s = t.Format("%H:%M:%S.%l");
                 wxSize ts = dc.GetTextExtent(s);
                 dc.DrawText( s, position - ts.GetX() / 2, sTimeScaleMinutesHeight );
             }
         }
     }

     // Draw video tracks
     // First determine starting point
     int y = mDividerPosition;
     BOOST_REVERSE_FOREACH( GuiTimeLineTrackPtr track, mVideoTracks )
     {
         y -= track->getBitmap().GetHeight();
     }
     BOOST_REVERSE_FOREACH( GuiTimeLineTrackPtr videoTrack, mVideoTracks)
     {
         dc.DrawBitmap(videoTrack->getBitmap(),wxPoint(0,y));
         y += videoTrack->getBitmap().GetHeight();
     }

     // Draw divider between video and audio tracks
     wxPen bluePen(*wxBLUE, 1);
     wxBrush audioVideoDividerBrush(wxColour(10,20,30),wxBRUSHSTYLE_CROSSDIAG_HATCH);
     dc.SetBrush(audioVideoDividerBrush);
     dc.SetPen(bluePen);
     dc.DrawRectangle(0,y,w,sAudioVideoDividerHeight);

     y += sAudioVideoDividerHeight;
     BOOST_FOREACH( GuiTimeLineTrackPtr audioTrack, mAudioTracks)
     {
         dc.DrawBitmap(audioTrack->getBitmap(),wxPoint(0,y));
         y += audioTrack->getBitmap().GetHeight();
     }
     Refresh(false);
 }


 wxBitmap GuiTimeLine::getDragBitmap(wxPoint& hotspot) //const
 {
     int w = mBitmap.GetWidth();
     int h = mBitmap.GetHeight();

     wxBitmap temp(w,h); // Create a bitmap equal in size to the entire virtual area (for simpler drawing code)
     wxBitmap mask(w,h,1);

     wxMemoryDC dc(temp); // Must go out of scope to be able to use temp.data below
     wxMemoryDC dcMask(mask);

     dc.SetBackground(*wxTRANSPARENT_BRUSH);
     dc.Clear();
     dc.SetBrush(*wxTRANSPARENT_BRUSH);
     dc.SetPen(*wxTRANSPARENT_PEN);
     dc.DrawRectangle(0,0,w,h);

     dcMask.SetBackground(*wxBLACK_BRUSH);
     dcMask.Clear();
     dcMask.SetPen(*wxWHITE_PEN);
     dcMask.SetBrush(*wxWHITE_BRUSH);

     // First determine starting point
     wxPoint position(0,mDividerPosition);
     BOOST_REVERSE_FOREACH( GuiTimeLineTrackPtr track, mVideoTracks )
     {
         position.y -= track->getBitmap().GetHeight();
     }

     // Draw video tracks
     BOOST_REVERSE_FOREACH( GuiTimeLineTrackPtr track, mVideoTracks)
     {
         track->drawClips(position,dc,dcMask);
         position.y += track->getBitmap().GetHeight();//trackDragBitmap.GetHeight();
     }

     // Draw audio tracks
     position.y += sAudioVideoDividerHeight;
     BOOST_FOREACH( GuiTimeLineTrackPtr track, mAudioTracks )
     {
         track->drawClips(position,dc,dcMask);
         position.y += track->getBitmap().GetHeight();//trackDragBitmap.GetHeight();
     }

     int origin_x = std::max(dcMask.MinX(),0); // todo constants
     int origin_y = std::max(dcMask.MinY(),0);
     int size_x = dcMask.MaxX() - origin_x;
     int size_y = dcMask.MaxY() - origin_y;

     dc.SelectObject(wxNullBitmap);
     dcMask.SelectObject(wxNullBitmap);

     temp.SetMask(new wxMask(mask));

     hotspot.x -= origin_x;
     hotspot.y -= origin_y;

     return temp.GetSubBitmap(wxRect(origin_x,origin_y,size_x,size_y));
 }

 //////////////////////////////////////////////////////////////////////////
 // SERIALIZATION 
 //////////////////////////////////////////////////////////////////////////

 template<class Archive>
 void GuiTimeLine::serialize(Archive & ar, const unsigned int version)
 {
     ar & mSequence;
     ar & mVideoTracks;
     ar & mAudioTracks;
     ar & mZoom;
     ar & mDividerPosition;
 }
 template void GuiTimeLine::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
 template void GuiTimeLine::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);


