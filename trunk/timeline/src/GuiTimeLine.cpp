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
#include "Drop.h"
#include "Zoom.h"
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

IMPLEMENTENUM(MouseOnClipPosition);

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

GuiTimeLine::GuiTimeLine(model::SequencePtr sequence)
:   wxScrolledWindow()
,   mMouseState(*this)
,   mWidth(0)
,   mHeight(0)
,   mDividerPosition(0)
,   mSequence(sequence)
{
    LOG_INFO;
}

void GuiTimeLine::init(wxWindow *parent)
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

    BOOST_FOREACH( model::TrackPtr track, mSequence->getVideoTracks())
    {
        GuiTimeLineTrack* p = new GuiTimeLineTrack(track);
        p->initTimeline(this);
        p->Bind(TRACK_UPDATE_EVENT, &GuiTimeLine::onTrackUpdated, this);
        // todo2 handle this via a OnVideoTrackAdded similar to the track handling of clip events from the model
    }
    BOOST_FOREACH( model::TrackPtr track, mSequence->getAudioTracks())
    {
        GuiTimeLineTrack* p = new GuiTimeLineTrack(track);
        p->initTimeline(this);
        p->Bind(TRACK_UPDATE_EVENT, &GuiTimeLine::onTrackUpdated, this);
        // todo2 handle this via a OnAudioTrackAdded similar to the track handling of clip events from the model
    }

    Bind(wxEVT_PAINT,               &GuiTimeLine::onPaint,              this);
    Bind(wxEVT_ERASE_BACKGROUND,    &GuiTimeLine::onEraseBackground,    this);
    Bind(wxEVT_SIZE,                &GuiTimeLine::onSize,               this);

    // From here on, processing continues with size events after laying out this widget.
}

GuiTimeLine::~GuiTimeLine()
{
    dynamic_cast<GuiWindow*>(wxGetApp().GetTopWindow())->getPreview().closeTimeline(this);
}

//////////////////////////////////////////////////////////////////////////
// PARTS OVER WHICH THE IMPLEMENTATION IS SPLIT
//////////////////////////////////////////////////////////////////////////

Zoom& GuiTimeLine::getZoom()
{ 
    return mZoom; 
}

const Zoom& GuiTimeLine::getZoom() const
{ 
    return mZoom; 
}

ViewMap& GuiTimeLine::getViewMap()
{ 
    return mViewMap; 
}

Intervals& GuiTimeLine::getIntervals()
{ 
    return mIntervals; 
}

MousePointer& GuiTimeLine::getMousepointer()
{ 
    return mMousePointer; 
}

Selection& GuiTimeLine::getSelection()
{ 
    return mSelection;
}

MenuHandler& GuiTimeLine::getMenuHandler()
{ 
    return mMenuHandler; 
}

Cursor& GuiTimeLine::getCursor()
{ 
    return mCursor; 
}

Drag& GuiTimeLine::getDrag()
{ 
    return mDrag; 
}

Drop& GuiTimeLine::getDrop()
{
    return mDrop;
}

//////////////////////////////////////////////////////////////////////////
// EVENTS
//////////////////////////////////////////////////////////////////////////

void GuiTimeLine::onSize(wxSizeEvent& event)
{
    determineHeight();

    mDividerPosition =
        Constants::sTimeScaleHeight +
        Constants::sMinimalGreyAboveVideoTracksHeight +
        (mHeight - Constants::sTimeScaleHeight - Constants::sMinimalGreyAboveVideoTracksHeight - Constants::sAudioVideoDividerHeight) / 2;

    updateSize(); // Triggers the initial drawing
}


void GuiTimeLine::onEraseBackground(wxEraseEvent& event)
{
    //event.Skip(); // The official way of doing it
}

void GuiTimeLine::onPaint( wxPaintEvent &WXUNUSED(event) )
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

void GuiTimeLine::onTrackUpdated( TrackUpdateEvent& event )
{
    LOG_INFO;
    getCursor().moveCursorOnUser(getCursor().getPosition()); // This is needed to reset iterators in model in case of clip addition/removal
    /** todo only redraw track */
    updateBitmap();
    Update();
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

PlayerPtr GuiTimeLine::getPlayer() const
{
    return mPlayer;
}

model::SequencePtr GuiTimeLine::getSequence() const
{
    return mSequence;
}

int GuiTimeLine::getWidth() const
{
    return mWidth;
}

int GuiTimeLine::getHeight() const
{
    return mHeight;
}

int GuiTimeLine::getDividerPosition() const
{
    return mDividerPosition;
}

wxPoint GuiTimeLine::getScrollOffset() const
{
    int scrollX, scrollY, ppuX, ppuY;
    GetViewStart(&scrollX,&scrollY);
    GetScrollPixelsPerUnit(&ppuX,&ppuY);
    return wxPoint(scrollX * ppuX, scrollY * ppuY);
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void GuiTimeLine::determineWidth()
{
    mWidth = std::max(std::max(
        mZoom.timeToPixels(5 * Constants::sMinute),            // Minimum width of 5 minutes
        mZoom.ptsToPixels(mSequence->getNumberOfFrames())),    // At least enough to hold all clips
        GetClientSize().GetWidth());                            // At least the widget size
}

void GuiTimeLine::determineHeight()
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
    determineWidth();
    determineHeight();

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

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void GuiTimeLine::serialize(Archive & ar, const unsigned int version)
{
    ar & mSequence;
    ar & mZoom;
    ar & mDividerPosition;
    ar & mIntervals;
}
template void GuiTimeLine::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void GuiTimeLine::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);

}} // namespace
