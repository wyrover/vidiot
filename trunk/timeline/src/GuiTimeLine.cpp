#include "GuiTimeLine.h"
#include <boost/make_shared.hpp>
#include <boost/foreach.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <wx/dcclient.h>
#include <wx/pen.h>
#include <wx/image.h>
#include <wx/bitmap.h>
#include <math.h>
#include <algorithm>
#include "Constants.h"
#include "UtilLog.h"
#include "GuiMain.h"
#include "GuiOptions.h"
#include "GuiPlayer.h"
#include "GuiPreview.h"
#include "GuiWindow.h"
#include "GuiTimeLineDropTarget.h"
#include "GuiTimeLineZoom.h"
#include "GuiTimeLineClip.h"
#include "GuiTimeLineTrack.h"
#include "UtilLogStl.h"
#include "AProjectViewNode.h"
#include "Sequence.h"
#include "VideoTrack.h"
#include "AudioTrack.h"
#include "ViewMap.h"
#include "ids.h"

namespace gui { namespace timeline {

DEFINE_EVENT(TIMELINE_CURSOR_MOVED, EventTimelineCursorMoved, long);

IMPLEMENTENUM(MouseOnClipPosition);

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

GuiTimeLine::GuiTimeLine(model::SequencePtr sequence)
:   wxScrolledWindow()
,   mZoom()
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
,   mDropArea(0,0,0,0)
,   mMenu()
,   mDragImage(0)
{
    LOG_INFO;

    mMenu.Append(ID_ADDVIDEOTRACK,  _("Add video track"));
    mMenu.Append(ID_ADDAUDIOTRACK,  _("Add audio track"));
    mMenu.AppendSeparator();
    mMenu.Append(ID_DELETEMARKED,   _("Delete marked regions from sequence"));
    mMenu.Append(ID_DELETEUNMARKED, _("Delete unmarked regions from sequence"));
    mMenu.Append(ID_REMOVEMARKERS,  _("Remove all markers"));
    // todo menusequence->Append(ID_CLOSESEQUENCE, _("Close"));

    //Bind(wxEVT_COMMAND_MENU_SELECTED,   &GuiTimeLine::OnCloseSequence,    this, ID_CLOSESEQUENCE);
    mMenu.Bind(wxEVT_COMMAND_MENU_SELECTED,   &GuiTimeLine::OnAddVideoTrack,    this, ID_ADDVIDEOTRACK);
    mMenu.Bind(wxEVT_COMMAND_MENU_SELECTED,   &GuiTimeLine::OnAddAudioTrack,    this, ID_ADDAUDIOTRACK);

    mZoom.initTimeline(this);
    mViewMap.initTimeline(this);
    mSelectIntervals.initTimeline(this);
    mMousePointer.initTimeline(this);
    mSelectClips.initTimeline(this);
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

    BOOST_FOREACH( model::TrackPtr track, mSequence->getVideoTracks())
    {
        GuiTimeLineTrack* p = new GuiTimeLineTrack(*this, mZoom, track);
        p->Bind(TRACK_UPDATE_EVENT, &GuiTimeLine::OnTrackUpdated, this);
        // todo2 handle this via a OnVideoTrackAdded similar to the track handling of clip events from the model
    }
    BOOST_FOREACH( model::TrackPtr track, mSequence->getAudioTracks())
    {
        GuiTimeLineTrack* p = new GuiTimeLineTrack(*this, mZoom, track);
        p->Bind(TRACK_UPDATE_EVENT, &GuiTimeLine::OnTrackUpdated, this);
        // todo2 handle this via a OnAudioTrackAdded similar to the track handling of clip events from the model
    }

    Bind(wxEVT_PAINT,               &GuiTimeLine::OnPaint,              this);
    Bind(wxEVT_ERASE_BACKGROUND,    &GuiTimeLine::OnEraseBackground,    this);
    Bind(wxEVT_SIZE,                &GuiTimeLine::OnSize,               this);

    ASSERT(mSequence);
    mPlayer = dynamic_cast<GuiWindow*>(wxGetApp().GetTopWindow())->getPreview().openTimeline(this);

    // From here on, processing continues with size events after laying out this widget.
}

GuiTimeLine::~GuiTimeLine()
{
    dynamic_cast<GuiWindow*>(wxGetApp().GetTopWindow())->getPreview().closeTimeline(this);

    Unbind(wxEVT_PAINT,               &GuiTimeLine::OnPaint,              this);
    Unbind(wxEVT_ERASE_BACKGROUND,    &GuiTimeLine::OnEraseBackground,    this);
    Unbind(wxEVT_SIZE,                &GuiTimeLine::OnSize,               this);
}

//////////////////////////////////////////////////////////////////////////
// SEQUENCE MENU
//////////////////////////////////////////////////////////////////////////

void GuiTimeLine::OnAddVideoTrack(wxCommandEvent& WXUNUSED(event))
{
    LOG_DEBUG;
    // todo handle this via timelinesview. that class is resp for maintaining the lst of sequences.
    //mProject->Submit(new command::TimelineCreateVideoTrack(*mOpenSequences.begin()));
}

void GuiTimeLine::OnAddAudioTrack(wxCommandEvent& WXUNUSED(event))
{
    LOG_DEBUG;
    // todo handle this via timelinesview. that class is resp for maintaining the lst of sequences.
    //    mProject->Submit(new command::TimelineCreateAudioTrack(*mOpenSequences.begin()));
}

//void GuiTimeLine::OnCloseSequence(wxCommandEvent& WXUNUSED(event))
//{
//    mTimelinesView->Close();
//}

//////////////////////////////////////////////////////////////////////////
// MODEL EVENTS
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// GUI EVENTS
//////////////////////////////////////////////////////////////////////////

void GuiTimeLine::OnSize(wxSizeEvent& event)
{
    DetermineHeight();

    mDividerPosition =
        Constants::sTimeScaleHeight +
        Constants::sMinimalGreyAboveVideoTracksHeight +
        (mHeight - Constants::sTimeScaleHeight - Constants::sMinimalGreyAboveVideoTracksHeight - Constants::sAudioVideoDividerHeight) / 2;

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

    wxPoint scroll = getScrollOffset();

    wxMemoryDC dcBmp(mBitmap);

    wxRegionIterator upd(GetUpdateRegion()); // get the update rect list
    while (upd)
    {
        int x = scroll.x + upd.GetX();
        int y = scroll.y + upd.GetY();
        int w = upd.GetW();
        int h = upd.GetH();
        dc.Blit(x,y,w,h,&dcBmp,x,y,wxCOPY,false,0,0);
        upd++;
    }

    // Draw marked areas
    getSelectIntervals().draw(dc);

    // Draw drop area
    if (!mDropArea.IsEmpty())
    {
        dc.SetPen(Constants::sDropAreaPen);
        dc.SetBrush(Constants::sDropAreaBrush);
        dc.DrawRectangle(mDropArea);
    }

    // Draw cursor
    dc.SetPen(Constants::sCursorPen);
    dc.DrawLine(wxPoint(mCursorPosition,0),wxPoint(mCursorPosition,mHeight));

}

//////////////////////////////////////////////////////////////////////////
// DRAWING EVENTS
//////////////////////////////////////////////////////////////////////////

void GuiTimeLine::OnTrackUpdated( TrackUpdateEvent& event )
{
    LOG_INFO;
    moveCursorOnUser(mCursorPosition); // This is needed to reset iterators in model in case of clip addition/removal
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

void GuiTimeLine::showDropArea(wxRect area)
{
    if (mDropArea != area)
    {
        mDropArea = area;
        Refresh(false);
        Update();

    }
}

wxMenu& GuiTimeLine::getMenu()
{
    return mMenu;
}

//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////

PlayerPtr GuiTimeLine::getPlayer() const
{
    return mPlayer;
}

//////////////////////////////////////////////////////////////////////////
// DRAGIMAGE
//////////////////////////////////////////////////////////////////////////

void GuiTimeLine::setDragImage(GuiTimeLineDragImage* dragimage)
{
    mDragImage = dragimage;
}

GuiTimeLineDragImage* GuiTimeLine::getDragImage() const
{
    return mDragImage;
}

//////////////////////////////////////////////////////////////////////////
// CURSOR
//////////////////////////////////////////////////////////////////////////

void GuiTimeLine::setCursorPosition(long position)
{
    VAR_DEBUG(mCursorPosition)(position);

    long oldPos = mCursorPosition;
    mCursorPosition = position;

    wxPoint scroll = getScrollOffset();

    // Refresh the old and new cursor position areas
    long cursorOnClientArea = mCursorPosition - scroll.x;
    long oldposOnClientArea = oldPos - scroll.x;
    RefreshRect(wxRect(std::min(cursorOnClientArea,oldposOnClientArea),0,std::abs(cursorOnClientArea-oldposOnClientArea)+1,mHeight),false);

    QueueEvent(new EventTimelineCursorMoved(mCursorPosition));
}

void GuiTimeLine::moveCursorOnPlayback(long pts)
{
    setCursorPosition(mZoom.ptsToPixels(pts));
}

void GuiTimeLine::moveCursorOnUser(int position)
{
    setCursorPosition(position);
    mPlayer->moveTo(mZoom.pixelsToPts(position));
}

//////////////////////////////////////////////////////////////////////////
// FROM COORDINATES TO OBJECTS
//////////////////////////////////////////////////////////////////////////

//todo maak een method die bepaalt wat er onder de cursor zit
//(Track + Clip .adjustBeginPoint.. en ook waar bij de Track/clip)
GuiTimeLineClip* GuiTimeLine::findClip(wxPoint p)
{
    model::TrackPtr track = findTrack(p.y).get<0>();
    if (track)
    {
        model::ClipPtr clip = track->getClip(mZoom.pixelsToPts(p.x));
        if (clip)
        {
            return getViewMap().getView(clip);
        }
    }
    return 0;
}

boost::tuple<model::TrackPtr,int> GuiTimeLine::findTrack(int yposition)
{
    int top = mDividerPosition;

    BOOST_REVERSE_FOREACH( model::TrackPtr track, mSequence->getVideoTracks() )
    {
        int bottom = top;
        top -= track->getHeight();
        if (yposition <= bottom && yposition >= top) return boost::make_tuple(track,top);
    }
    int bottom = mDividerPosition + Constants::sAudioVideoDividerHeight;
    BOOST_FOREACH( model::TrackPtr track, mSequence->getAudioTracks() )
    {
        int top = bottom;
        bottom += track->getHeight();
        if (yposition <= bottom && yposition >= top) return boost::make_tuple(track,top);
    }
    return boost::make_tuple(model::TrackPtr(),0);
}

PointerPositionInfo GuiTimeLine::getPointerInfo(wxPoint pointerposition)
{
    PointerPositionInfo info;
    info.track = model::TrackPtr();
    info.clip = model::ClipPtr();
    info.trackPosition = 0;

    // Find possible videotrack under pointer
    int top = mDividerPosition;
    BOOST_REVERSE_FOREACH( model::TrackPtr track, mSequence->getVideoTracks() )
    {
        int bottom = top;
        top -= track->getHeight();
        if (pointerposition.y <= bottom && pointerposition.y >= top)
        {
            info.track = track;
            info.trackPosition = top;
            break;
        }
    }
    if (!info.track)
    {
        // Find possible audiotrack under pointer
        int bottom = mDividerPosition + Constants::sAudioVideoDividerHeight;
        BOOST_FOREACH( model::TrackPtr track, mSequence->getAudioTracks() )
        {
            int top = bottom;
            bottom += track->getHeight();
            if (pointerposition.y <= bottom && pointerposition.y >= top)
            {
                info.track = track;
                info.trackPosition = top;
                break;
            }
        }
    }

    // Find clip under pointer
    if (info.track)
    {
        info.clip = info.track->getClip(mZoom.pixelsToPts(pointerposition.x));
    }

    // Find logical position of pointer wrt clips
    if (info.clip)
    {
        // This is handled on a per-pixel and not per-pts basis. That ensures
        // that this still works for clips which are very small when zoomed out.
        // (then the cursor won't flip too much).
        GuiTimeLineClip* clip = getViewMap().getView(info.clip);
        int dist_begin = pointerposition.x - clip->getLeftPosition();
        int dist_end = clip->getRightPosition() - pointerposition.x;

        if (dist_begin <= 1)
        {
            // Possibly between clips. However, this is only relevant if there
            // is another nonempty clip adjacent to this clip.
            model::ClipPtr previous = info.track->getPreviousClip(info.clip);
            info.logicalclipposition = (!previous || previous->isA<model::EmptyClip>()) ? ClipBegin : ClipBetween;
        }
        else if (dist_end <= 1)
        {
            // Possibly between clips. However, this is only relevant if there
            // is another nonempty clip adjacent to this clip.
            model::ClipPtr next = info.track->getNextClip(info.clip);
            info.logicalclipposition = (!next || next->isA<model::EmptyClip>()) ? ClipEnd : ClipBetween;
        }
        else if ((dist_begin > 1) && (dist_begin < 4))
        {
            info.logicalclipposition = ClipBegin;
        }
        else if ((dist_end > 1) && (dist_end < 4))
        {
            info.logicalclipposition = ClipEnd;
        }
        else
        {
            info.logicalclipposition = ClipInterior;
        }
    }
    return info;
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void GuiTimeLine::DetermineWidth()
{
    mWidth = std::max(std::max(
        mZoom.timeToPixels(5 * Constants::sMinute),            // Minimum width of 5 minutes
        mZoom.ptsToPixels(mSequence->getNumberOfFrames())),    // At least enough to hold all clips
        GetClientSize().GetWidth());                            // At least the widget size
}

void GuiTimeLine::DetermineHeight()
{
    int requiredHeight = Constants::sTimeScaleHeight;
    requiredHeight += Constants::sMinimalGreyAboveVideoTracksHeight;
    BOOST_REVERSE_FOREACH( model::TrackPtr track, mSequence->getVideoTracks())
    {
        requiredHeight += track->getHeight();
    }
    requiredHeight += Constants::sAudioVideoDividerHeight;
    BOOST_FOREACH( model::TrackPtr track, mSequence->getVideoTracks() )
    {
        requiredHeight += track->getHeight();
    }
    requiredHeight += Constants::sMinimalGreyBelowAudioTracksHeight;

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
    wxMemoryDC dc(mBitmap);

    // Get size of canvas
    int w = mBitmap.GetWidth();
    int h = mBitmap.GetHeight();

    // Set BG
    dc.SetPen(Constants::sBackgroundPen);
    dc.SetBrush(Constants::sBackgroundBrush);
    dc.DrawRectangle(0,0,w,h);

    // Draw timescale
    dc.SetBrush(wxNullBrush);
    dc.SetPen(Constants::sTimeScaleDividerPen);
    dc.DrawRectangle(0,0,w,Constants::sTimeScaleHeight);

    dc.SetFont(*Constants::sTimeScaleFont);

    // Draw seconds and minutes lines
    for (int ms = 0; mZoom.timeToPixels(ms) <= w; ms += Constants::sSecond)
    {
        int position = mZoom.timeToPixels(ms);
        bool isMinute = (ms % Constants::sMinute == 0);
        int height = Constants::sTimeScaleSecondHeight;

        if (isMinute)
        {
            height = Constants::sTimeScaleMinutesHeight;
        }

        dc.DrawLine(position,0,position,height);

        if (ms == 0)
        {
            dc.DrawText( "0", 5, Constants::sTimeScaleMinutesHeight );
        }
        else
        {
            if (isMinute)
            {
                wxDateTime t(ms / Constants::sHour, (ms % Constants::sHour) / Constants::sMinute, (ms % Constants::sMinute) / Constants::sSecond, ms % Constants::sSecond);
                wxString s = t.Format("%H:%M:%S.%l");
                wxSize ts = dc.GetTextExtent(s);
                dc.DrawText( s, position - ts.GetX() / 2, Constants::sTimeScaleMinutesHeight );
            }
        }
    }

    // Draw video tracks
    // First determine starting point
    int y = mDividerPosition;
    BOOST_REVERSE_FOREACH( model::TrackPtr track, mSequence->getVideoTracks() )
    {
        y -= track->getHeight();
    }
    BOOST_REVERSE_FOREACH( model::TrackPtr track, mSequence->getVideoTracks())
    {
        wxBitmap b = getViewMap().getView(track)->getBitmap();
        dc.DrawBitmap(b,wxPoint(0,y));
        y += track->getHeight();
    }

    // Draw divider between video and audio tracks
    dc.SetBrush(Constants::sAudioVideoDividerBrush);
    dc.SetPen(Constants::sAudioVideoDividerPen);
    dc.DrawRectangle(0,y,w,Constants::sAudioVideoDividerHeight);

    y += Constants::sAudioVideoDividerHeight;
    BOOST_FOREACH( model::TrackPtr track, mSequence->getAudioTracks() )
    {
        wxBitmap b = getViewMap().getView(track)->getBitmap();
        dc.DrawBitmap(b,wxPoint(0,y));
        y += track->getHeight();
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
    BOOST_REVERSE_FOREACH( model::TrackPtr track, mSequence->getVideoTracks() )
    {
        position.y -= track->getHeight();
    }

    // Draw video tracks
    BOOST_REVERSE_FOREACH( model::TrackPtr track, mSequence->getVideoTracks() )
    {
        getViewMap().getView(track)->drawClips(position,dc,dcMask);
        position.y += track->getHeight();
    }

    // Draw audio tracks
    position.y += Constants::sAudioVideoDividerHeight;
    BOOST_FOREACH( model::TrackPtr track, mSequence->getAudioTracks() )
    {
        getViewMap().getView(track)->drawClips(position,dc,dcMask);
        position.y += track->getHeight();
    }

    int origin_x = std::max(dcMask.MinX(),0);
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

wxPoint GuiTimeLine::getScrollOffset() const
{
    int scrollX, scrollY, ppuX, ppuY;
    GetViewStart(&scrollX,&scrollY);
    GetScrollPixelsPerUnit(&ppuX,&ppuY);
    return wxPoint(scrollX * ppuX, scrollY * ppuY);
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void GuiTimeLine::serialize(Archive & ar, const unsigned int version)
{
    ar & mSequence;
    ar & mZoom;
    ar & mDividerPosition;
    ar & mSelectIntervals;
}
template void GuiTimeLine::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void GuiTimeLine::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);

}} // namespace
