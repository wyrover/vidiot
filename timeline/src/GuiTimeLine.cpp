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
#include <wx/image.h>
#include <wx/bitmap.h>
#include <math.h>
#include <algorithm>
#include "Constants.h"
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

namespace gui { namespace timeline {

DEFINE_EVENT(TIMELINE_CURSOR_MOVED, EventTimelineCursorMoved, long);

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
,   mDropArea(0,0,0,0)
,   mSelectedIntervals()
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
	    mSelectedIntervals = boost::make_shared<SelectIntervals>();
    }
}

void GuiTimeLine::init(wxWindow *parent)
{
    Create(parent,wxID_ANY,wxPoint(0,0),wxDefaultSize,wxHSCROLL|wxVSCROLL|wxSUNKEN_BORDER);

    SetScrollRate( 10, 10 );
    EnableScrolling(true,true);
    SetBackgroundColour(* wxLIGHT_GREY);
    SetDropTarget(new GuiTimeLineDropTarget(mZoom,this));

    // Initialize all helper objects
    mSelectedIntervals->init(this);

    // Must be done before initializing tracks, since tracks derive their width from the entire timeline
    DetermineWidth();

    // Initialize tracks (this also creates the bitmaps).
    // Furthermore, the list of all clips is passed in order to 
    // make links between clips.
    GuiTimeLineClips allclips = getClips();
    BOOST_REVERSE_FOREACH( GuiTimeLineTrackPtr track, mVideoTracks )
    {
        track->init(this, allclips);
    }
    BOOST_FOREACH( GuiTimeLineTrackPtr track, mAudioTracks )
    {
        track->init(this, allclips);
    }

    Bind(wxEVT_PAINT,               &GuiTimeLine::OnPaint,              this);
    Bind(wxEVT_ERASE_BACKGROUND,    &GuiTimeLine::OnEraseBackground,    this);
    Bind(wxEVT_SIZE,                &GuiTimeLine::OnSize,               this);

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
    dynamic_cast<GuiWindow*>(wxGetApp().GetTopWindow())->getPreview().closeTimeline(this);

    Unbind(wxEVT_PAINT,               &GuiTimeLine::OnPaint,              this);
    Unbind(wxEVT_ERASE_BACKGROUND,    &GuiTimeLine::OnEraseBackground,    this);
    Unbind(wxEVT_SIZE,                &GuiTimeLine::OnSize,               this);

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
    mSelectedIntervals->draw(dc);

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

int GuiTimeLine::getIndex(GuiTimeLineTrackPtr track) const
{
    int index = 0;
    BOOST_FOREACH( GuiTimeLineTrackPtr t, mVideoTracks )
    {
        index++;
        if (t == track)
        {
            return index;
        }
    }
    index = 0;
    BOOST_FOREACH( GuiTimeLineTrackPtr t, mAudioTracks )
    {
        index--;
        if (t == track)
        {
            return index;
        }
    }
    return index;
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


boost::tuple<GuiTimeLineClipPtr,int> GuiTimeLine::findClip(wxPoint p) const
{
    GuiTimeLineTrackPtr track = findTrack(p.y).get<0>();
    if (track)
    {
        return track->findClip(p.x);
    }
    return boost::make_tuple(GuiTimeLineClipPtr(),0);
}

boost::tuple<GuiTimeLineTrackPtr,int> GuiTimeLine::findTrack(int yposition) const
{
    int top = mDividerPosition;
    BOOST_REVERSE_FOREACH( GuiTimeLineTrackPtr track, mVideoTracks )
    {
        int bottom = top;
        top -= track->getBitmap().GetHeight();
        if (yposition <= bottom && yposition >= top) return boost::make_tuple(track,top);
    }
    int bottom = mDividerPosition + Constants::sAudioVideoDividerHeight;
    BOOST_FOREACH( GuiTimeLineTrackPtr track, mAudioTracks )
    {
        int top = bottom;
        bottom += track->getBitmap().GetHeight();
        if (yposition <= bottom && yposition >= top) return boost::make_tuple(track,top);
    }
    return boost::make_tuple(GuiTimeLineTrackPtr(),0);
}

GuiTimeLineTracks GuiTimeLine::getTracks() const
{
    GuiTimeLineTracks tracks;
    BOOST_FOREACH( GuiTimeLineTrackPtr track, mVideoTracks )
    {
        tracks.push_back(track);
    }
    BOOST_FOREACH( GuiTimeLineTrackPtr track, mAudioTracks )
    {
        tracks.push_back(track);
    }
    return tracks;
}

GuiTimeLineClips GuiTimeLine::getClips() const
{
    GuiTimeLineClips clips;
    BOOST_FOREACH( GuiTimeLineTrackPtr track, mVideoTracks )
    {
        clips.splice(clips.begin(), track->getClips());
    }
    BOOST_FOREACH( GuiTimeLineTrackPtr track, mAudioTracks )
    {
        clips.splice(clips.begin(), track->getClips());
    }
    return clips;
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
    int requiredHeight = Constants::sTimeScaleHeight;
    requiredHeight += Constants::sMinimalGreyAboveVideoTracksHeight;
    BOOST_REVERSE_FOREACH( GuiTimeLineTrackPtr track, mVideoTracks )
    {
        requiredHeight += track->getBitmap().GetHeight();
    }
    requiredHeight += Constants::sAudioVideoDividerHeight;
    BOOST_FOREACH( GuiTimeLineTrackPtr track, mAudioTracks )
    {
        requiredHeight += track->getBitmap().GetHeight();
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
    for (int ms = 0; mZoom->timeToPixels(ms) <= w; ms += Constants::sSecond)
    {
        int position = mZoom->timeToPixels(ms);
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
    dc.SetBrush(Constants::sAudioVideoDividerBrush);
    dc.SetPen(Constants::sAudioVideoDividerPen);
    dc.DrawRectangle(0,y,w,Constants::sAudioVideoDividerHeight);

    y += Constants::sAudioVideoDividerHeight;
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
    position.y += Constants::sAudioVideoDividerHeight;
    BOOST_FOREACH( GuiTimeLineTrackPtr track, mAudioTracks )
    {
        track->drawClips(position,dc,dcMask);
        position.y += track->getBitmap().GetHeight();//trackDragBitmap.GetHeight();
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
    ar & mVideoTracks;
    ar & mAudioTracks;
    ar & mZoom;
    ar & mDividerPosition;
    ar & mSelectedIntervals;
}
template void GuiTimeLine::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void GuiTimeLine::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);

}} // namespace
