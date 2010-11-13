#include "Timeline.h"
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
#include "Drop.h"
#include "Zoom.h"
#include "ClipView.h"
#include "TrackView.h"
#include "UtilLogStl.h"
#include "AProjectViewNode.h"
#include "Sequence.h"
#include "VideoTrack.h"
#include "AudioTrack.h"
#include "ViewMap.h"
#include "ids.h"

namespace gui { namespace timeline {

IMPLEMENTENUM(MouseOnClipPosition);

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

Timeline::Timeline(model::SequencePtr sequence)
:   wxScrolledWindow()
,   mMouseState(*this)
,   mWidth(0)
,   mHeight(0)
,   mDividerPosition(0)
,   mSequence(sequence)
{
    LOG_INFO;
}

void Timeline::init(wxWindow *parent)
{
    ASSERT(mSequence);
    mPlayer = dynamic_cast<GuiWindow*>(wxGetApp().GetTopWindow())->getPreview().openTimeline(this);

    mZoom.initTimeline(this);
    mViewMap.initTimeline(this);
    mIntervals.initTimeline(this);
    mMousePointer.initTimeline(this);
    mSelection.initTimeline(this);
    mCursor.initTimeline(this);
    mDrag.initTimeline(this);
    mDrop.initTimeline(this);
    mMenuHandler.initTimeline(this); // Init as last since it depends on other parts

    Create(parent,wxID_ANY,wxPoint(0,0),wxDefaultSize,wxHSCROLL|wxVSCROLL|wxSUNKEN_BORDER);

    SetScrollRate( 10, 10 );
    EnableScrolling(true,true);
    SetBackgroundColour(* wxLIGHT_GREY);

    // Must be done before initializing tracks, since tracks derive their width from the entire timeline
    determineWidth();

    model::TrackChange videoTracks(mSequence->getVideoTracks());
    onVideoTracksAdded(model::EventAddVideoTracks(videoTracks));
    
    //BOOST_FOREACH( model::TrackPtr track, mSequence->getVideoTracks())
    //{
    //    TrackView* p = new TrackView(track);
    //    p->initTimeline(this);
    //    p->Bind(TRACK_UPDATE_EVENT, &Timeline::onTrackUpdated, this);
    //    // todo2 handle this via a OnVideoTrackAdded similar to the track handling of clip events from the model
    //}
    BOOST_FOREACH( model::TrackPtr track, mSequence->getAudioTracks())
    {
        TrackView* p = new TrackView(track);
        p->initTimeline(this);
        p->Bind(TRACK_UPDATE_EVENT, &Timeline::onTrackUpdated, this);
        // todo2 handle this via a OnAudioTrackAdded similar to the track handling of clip events from the model
    }

    Bind(wxEVT_PAINT,               &Timeline::onPaint,              this);
    Bind(wxEVT_ERASE_BACKGROUND,    &Timeline::onEraseBackground,    this);
    Bind(wxEVT_SIZE,                &Timeline::onSize,               this);

    mSequence->Bind(model::EVENT_ADD_VIDEO_TRACK,       &Timeline::onVideoTracksAdded,    this);
    mSequence->Bind(model::EVENT_REMOVE_VIDEO_TRACK,    &Timeline::onVideoTracksRemoved,  this);
    mSequence->Bind(model::EVENT_ADD_AUDIO_TRACK,       &Timeline::onAudioTracksAdded,   this);
    mSequence->Bind(model::EVENT_REMOVE_AUDIO_TRACK,    &Timeline::onAudioTracksRemoved,  this);

    // From here on, processing continues with size events after laying out this widget.
}

Timeline::~Timeline()
{
    dynamic_cast<GuiWindow*>(wxGetApp().GetTopWindow())->getPreview().closeTimeline(this);
}

//////////////////////////////////////////////////////////////////////////
// PARTS OVER WHICH THE IMPLEMENTATION IS SPLIT
//////////////////////////////////////////////////////////////////////////

Zoom& Timeline::getZoom()
{ 
    return mZoom; 
}

const Zoom& Timeline::getZoom() const
{ 
    return mZoom; 
}

ViewMap& Timeline::getViewMap()
{ 
    return mViewMap; 
}

Intervals& Timeline::getIntervals()
{ 
    return mIntervals; 
}

MousePointer& Timeline::getMousepointer()
{ 
    return mMousePointer; 
}

Selection& Timeline::getSelection()
{ 
    return mSelection;
}

MenuHandler& Timeline::getMenuHandler()
{ 
    return mMenuHandler; 
}

Cursor& Timeline::getCursor()
{ 
    return mCursor; 
}

Drag& Timeline::getDrag()
{ 
    return mDrag; 
}

Drop& Timeline::getDrop()
{
    return mDrop;
}

//////////////////////////////////////////////////////////////////////////
// GUI EVENTS
//////////////////////////////////////////////////////////////////////////

void Timeline::onSize(wxSizeEvent& event)
{
    determineHeight();

    mDividerPosition =
        Constants::sTimeScaleHeight +
        Constants::sMinimalGreyAboveVideoTracksHeight +
        (mHeight - Constants::sTimeScaleHeight - Constants::sMinimalGreyAboveVideoTracksHeight - Constants::sAudioVideoDividerHeight) / 2;

    updateSize(); // Triggers the initial drawing
}

void Timeline::onEraseBackground(wxEraseEvent& event)
{
    //event.Skip(); // The official way of doing it
}

void Timeline::onPaint( wxPaintEvent &WXUNUSED(event) )
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

    getIntervals().draw(dc);
    getDrop().draw(dc);
    getCursor().draw(dc);

}

void Timeline::onTrackUpdated( TrackUpdateEvent& event )
{
    LOG_INFO;
    getCursor().moveCursorOnUser(getCursor().getPosition()); // This is needed to reset iterators in model in case of clip addition/removal
    /** todo only redraw track */
    updateBitmap();
    Update();
}

//////////////////////////////////////////////////////////////////////////
// MODEL EVENTS
//////////////////////////////////////////////////////////////////////////

void Timeline::onVideoTracksAdded( model::EventAddVideoTracks& event )
{
    BOOST_FOREACH( model::TrackPtr track, event.getValue().addedTracks)
    {
        TrackView* p = new TrackView(track);
        p->initTimeline(this);
        p->Bind(TRACK_UPDATE_EVENT, &Timeline::onTrackUpdated, this);
        // todo2 handle this via a OnVideoTrackAdded similar to the track handling of clip events from the model
    }
}

void Timeline::onVideoTracksRemoved( model::EventRemoveVideoTracks& event )
{

}

void Timeline::onAudioTracksAdded( model::EventAddAudioTracks& event )
{

}

void Timeline::onAudioTracksRemoved( model::EventRemoveAudioTracks& event )
{

}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

PlayerPtr Timeline::getPlayer() const
{
    return mPlayer;
}

model::SequencePtr Timeline::getSequence() const
{
    return mSequence;
}

int Timeline::getWidth() const
{
    return mWidth;
}

int Timeline::getHeight() const
{
    return mHeight;
}

int Timeline::getDividerPosition() const
{
    return mDividerPosition;
}

wxPoint Timeline::getScrollOffset() const
{
    int scrollX, scrollY, ppuX, ppuY;
    GetViewStart(&scrollX,&scrollY);
    GetScrollPixelsPerUnit(&ppuX,&ppuY);
    return wxPoint(scrollX * ppuX, scrollY * ppuY);
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void Timeline::determineWidth()
{
    mWidth = std::max(std::max(
        mZoom.timeToPixels(5 * Constants::sMinute),            // Minimum width of 5 minutes
        mZoom.ptsToPixels(mSequence->getNumberOfFrames())),    // At least enough to hold all clips
        GetClientSize().GetWidth());                            // At least the widget size
}

void Timeline::determineHeight()
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

void Timeline::updateSize()
{
    determineWidth();
    determineHeight();

    SetVirtualSize(mWidth,mHeight);
    mBitmap.Create(mWidth,mHeight);

    updateBitmap();
}

void Timeline::updateBitmap()
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

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void Timeline::serialize(Archive & ar, const unsigned int version)
{
    ar & mSequence;
    ar & mZoom;
    ar & mDividerPosition;
    ar & mIntervals;
}
template void Timeline::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void Timeline::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);

}} // namespace
