#include "Timeline.h"
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
#include "UtilLogStl.h"
#include "Sequence.h"
#include "VideoTrack.h"
#include "AudioTrack.h"
#include "Intervals.h"
#include "Selection.h"
#include "MousePointer.h"
#include "Cursor.h"
#include "Drag.h"
#include "Menu.h"
#include "Zoom.h"
#include "State.h"
#include "ViewMap.h"
#include "Drop.h"
#include "VideoView.h"
#include "AudioView.h"
#include "ViewMap.h"
#include "ids.h"
#include "UtilSerializeWxwidgets.h"

namespace gui { namespace timeline {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

Timeline::Timeline(model::SequencePtr sequence)
:   wxScrolledWindow()
,   mZoom(this)
,   mViewMap(0)
,   mIntervals(this)
,   mMousePointer(0)
,   mSelection(0)
,   mMenuHandler(0)
,   mCursor(0)
,   mDrag(0)
,   mDrop(0)
,   mVideoView(0)
,   mAudioView(0)
,   mMouseState(0)
,   mWidth(0)
,   mHeight(0)
,   mDividerPosition(0)
,   mRedrawOnIdle(false)
,   mSequence(sequence)
{
    LOG_INFO;

    //mZoom = new Zoom(this);

}

void Timeline::init(wxWindow *parent)
{
    Create(parent,wxID_ANY,wxPoint(0,0),wxDefaultSize,wxHSCROLL|wxVSCROLL|wxSUNKEN_BORDER);
    SetScrollRate( 10, 10 );
    EnableScrolling(true,true);
    SetBackgroundColour(*wxLIGHT_GREY);

    ASSERT(mSequence);

    mPlayer = dynamic_cast<GuiWindow*>(wxGetApp().GetTopWindow())->getPreview().openTimeline(this);

    mViewMap = new ViewMap(this);
    //mIntervals = new Intervals(this);
    mMousePointer = new MousePointer(this);
    mSelection = new Selection(this);
    mCursor = new Cursor(this); // Must be AFTER mPlayer
    mDrag = new Drag(this);
    mDrop = new Drop(this);
    mVideoView = new VideoView(this);
    mAudioView = new AudioView(this);
    mMouseState = new state::Machine(*this); // Must be AFTER mViewMap
    mMenuHandler = new MenuHandler(this); // Init as last since it depends on other parts

    mRedrawOnIdle = true;

    Bind(wxEVT_IDLE,                &Timeline::onIdle,               this);
    Bind(wxEVT_PAINT,               &Timeline::onPaint,              this);
    Bind(wxEVT_ERASE_BACKGROUND,    &Timeline::onEraseBackground,    this);
    Bind(wxEVT_SIZE,                &Timeline::onSize,               this);

    updateSize();

    // From here on, processing continues with size events after laying out this widget.
}

Timeline::~Timeline()
{
    dynamic_cast<GuiWindow*>(wxGetApp().GetTopWindow())->getPreview().closeTimeline(this);
}

//////////////////////////////////////////////////////////////////////////
// PART
//////////////////////////////////////////////////////////////////////////

Timeline& Timeline::getTimeline()
{
    return *this;
}

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
    return *mViewMap; 
}

Intervals& Timeline::getIntervals()
{ 
    return mIntervals; 
}

MousePointer& Timeline::getMousepointer()
{ 
    return *mMousePointer; 
}

Selection& Timeline::getSelection()
{ 
    return *mSelection;
}

MenuHandler& Timeline::getMenuHandler()
{ 
    return *mMenuHandler; 
}

Cursor& Timeline::getCursor()
{ 
    return *mCursor; 
}

Drag& Timeline::getDrag()
{ 
    return *mDrag; 
}

Drop& Timeline::getDrop()
{
    return *mDrop;
}

VideoView& Timeline::getVideoView()
{
    return *mVideoView;
}

AudioView& Timeline::getAudioView()
{
    return *mAudioView;
}

//////////////////////////////////////////////////////////////////////////
// GUI EVENTS
//////////////////////////////////////////////////////////////////////////

void Timeline::onIdle(wxIdleEvent& event)
{
    if (mRedrawOnIdle)
    {
        // This is done to avoid using intermediary states for iterating though
        // the model. For instance, when replacing clips with other clips, first
        // a unregisterView event and then a registerView event is received. However, while 
        // receiving the unregisterView event, the actual adding may already have been
        // done. Then the view for the added clips has not yet been initialized.
        updateBitmap();
        mRedrawOnIdle = false;
    }
    event.Skip();
}

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
    LOG_INFO;
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
        VAR_DEBUG(x)(y)(w)(h);
        dc.Blit(x,y,w,h,&dcBmp,x,y,wxCOPY,false,0,0);
        upd++;
    }

    getIntervals().draw(dc);
    getDrag().draw(dc);
    getDrop().draw(dc);
    getCursor().draw(dc);
}

//////////////////////////////////////////////////////////////////////////
// PROPAGATE UPDATES UPWARD
//////////////////////////////////////////////////////////////////////////

void Timeline::onViewUpdated( ViewUpdateEvent& event )
{
    mRedrawOnIdle = true;
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
    mWidth = getVideoView().requiredWidth(); /** @todo determine from seuqence */
}

void Timeline::determineHeight()
{
    int requiredHeight = Constants::sTimeScaleHeight;
    requiredHeight += Constants::sMinimalGreyAboveVideoTracksHeight;
    requiredHeight += getVideoView().requiredHeight();
    requiredHeight += Constants::sAudioVideoDividerHeight;
    requiredHeight += getAudioView().requiredHeight();
    requiredHeight += Constants::sMinimalGreyBelowAudioTracksHeight;
    mHeight = std::max(requiredHeight, GetClientSize().GetHeight());
}

void Timeline::updateSize()
{
    determineWidth();
    determineHeight();

    SetVirtualSize(mWidth,mHeight);
    mBitmap.Create(mWidth,mHeight);

    mRedrawOnIdle = true;
}

void Timeline::updateBitmap()
{
    LOG_DEBUG;
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
    for (int ms = 0; getZoom().timeToPixels(ms) <= w; ms += Constants::sSecond)
    {
        int position = getZoom().timeToPixels(ms);
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

    const wxBitmap& videotracks = getVideoView().getBitmap();
    dc.DrawBitmap(videotracks,wxPoint(0,mDividerPosition - videotracks.GetHeight()));

    dc.SetBrush(*wxTRANSPARENT_BRUSH);
    dc.SetPen(*wxWHITE_PEN);
    dc.DrawRectangle(0,mDividerPosition - videotracks.GetHeight(),w,videotracks.GetHeight());

    // Draw divider between video and audio tracks
    dc.SetBrush(Constants::sAudioVideoDividerBrush);
    dc.SetPen(Constants::sAudioVideoDividerPen);
    dc.DrawRectangle(0,mDividerPosition,w,Constants::sAudioVideoDividerHeight);

    const wxBitmap& audiotracks = getAudioView().getBitmap();
    dc.DrawBitmap(audiotracks,wxPoint(0,mDividerPosition + Constants::sAudioVideoDividerHeight));

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
    //if (Archive::is_loading::value)
    //{
    //    wxRegion intervals;
    //    ar & intervals;
    //    mIntervals->set(intervals);
    //}
    //else
    //{
    //    ar & mIntervals->get();
    //}
}
template void Timeline::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void Timeline::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);

}} // namespace
