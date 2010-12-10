#include "Timeline.h"
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <wx/dcclient.h>
#include <algorithm>
#include <wx/dcmemory.h>
#include "Constants.h"
#include "Layout.h"
#include "UtilLog.h"
#include "GuiMain.h"
#include "GuiOptions.h"
#include "GuiPlayer.h"
#include "GuiPreview.h"
#include "GuiWindow.h"
#include "Intervals.h"
#include "Selection.h"
#include "MousePointer.h"
#include "Cursor.h"
#include "Drag.h"
#include "Divider.h"
#include "Menu.h"
#include "Project.h"
#include "Zoom.h"
#include "Sequence.h"
#include "State.h"
#include "ViewMap.h"
#include "Drop.h"
#include "UtilLogWxwidgets.h"
#include "ViewMap.h"
#include "VideoView.h"
#include "AudioView.h"

namespace gui { namespace timeline {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

Timeline::Timeline(wxWindow *parent, model::SequencePtr sequence)
:   wxScrolledWindow(parent,wxID_ANY,wxPoint(0,0),wxDefaultSize,wxHSCROLL|wxVSCROLL|wxSUNKEN_BORDER)
,   View(this) // Has itself as parent...
//////////////////////////////////////////////////////////////////////////
,   mSequence(sequence)
,   mPlayer(dynamic_cast<GuiWindow*>(wxGetApp().GetTopWindow())->getPreview().openTimeline(sequence,this))
,   mRedrawOnIdle(true)
//////////////////////////////////////////////////////////////////////////
,   mZoom(new Zoom(this))
,   mViewMap(new ViewMap(this))
,   mIntervals(new Intervals(this))
,   mMousePointer(new MousePointer(this))
,   mSelection(new Selection(this))
,   mCursor(new Cursor(this))
,   mDrag(new Drag(this))
,   mDrop(new Drop(this))
,   mDivider(new Divider(this))
,   mMouseState(new state::Machine(*this))
,   mMenuHandler(new MenuHandler(this))
//////////////////////////////////////////////////////////////////////////
,   mVideoView(new VideoView(this))
,   mAudioView(new AudioView(this))
//////////////////////////////////////////////////////////////////////////
{
    LOG_INFO;

    SetScrollRate( 10, 10 );
    EnableScrolling(true,true);

    // To ensure that for newly opened timelines the initial position is ok
    // (should take 'minimum position' into account). This can only be done
    // after both mDivider AND mVideoView are initialized.
    getDivider().setPosition(getDivider().getPosition());

    Bind(wxEVT_IDLE,                &Timeline::onIdle,               this);
    Bind(wxEVT_PAINT,               &Timeline::onPaint,              this);
    Bind(wxEVT_ERASE_BACKGROUND,    &Timeline::onEraseBackground,    this);
    Bind(wxEVT_SIZE,                &Timeline::onSize,               this);

    // From here on, processing continues after the next idle event.
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

const Timeline& Timeline::getTimeline() const
{
    return *this;
}

Zoom& Timeline::getZoom()
{ 
    return *mZoom; 
}

const Zoom& Timeline::getZoom() const
{ 
    return *mZoom; 
}

ViewMap& Timeline::getViewMap()
{ 
    return *mViewMap; 
}

const ViewMap& Timeline::getViewMap() const
{ 
    return *mViewMap; 
}

Intervals& Timeline::getIntervals()
{ 
    return *mIntervals; 
}

const Intervals& Timeline::getIntervals() const
{ 
    return *mIntervals; 
}

MousePointer& Timeline::getMousepointer()
{ 
    return *mMousePointer; 
}

const MousePointer& Timeline::getMousepointer() const
{ 
    return *mMousePointer; 
}

Selection& Timeline::getSelection()
{ 
    return *mSelection;
}

const Selection& Timeline::getSelection() const
{ 
    return *mSelection;
}

MenuHandler& Timeline::getMenuHandler()
{ 
    return *mMenuHandler; 
}

const MenuHandler& Timeline::getMenuHandler() const
{ 
    return *mMenuHandler; 
}

Cursor& Timeline::getCursor()
{ 
    return *mCursor; 
}

const Cursor& Timeline::getCursor() const
{ 
    return *mCursor; 
}

Drag& Timeline::getDrag()
{ 
    return *mDrag; 
}

const Drag& Timeline::getDrag() const
{ 
    return *mDrag; 
}

Drop& Timeline::getDrop()
{
    return *mDrop;
}

const Drop& Timeline::getDrop() const
{
    return *mDrop;
}

Divider& Timeline::getDivider()
{
    return *mDivider;
}

const Divider& Timeline::getDivider() const
{
    return *mDivider;
}

model::SequencePtr Timeline::getSequence()
{
    return mSequence;
}

const model::SequencePtr Timeline::getSequence() const
{
    return mSequence;
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
        Refresh(false);
        mRedrawOnIdle = false;
    }
    event.Skip();
}

void Timeline::onSize(wxSizeEvent& event)
{
    // See onViewUpdated.
    // This invalidation causes that event, resulting in a resize.
    invalidateBitmap();
    //SetVirtualSize(requiredWidth(),requiredHeight());
    //mRedrawOnIdle = true;
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

    wxBitmap bitmap = getBitmap();
    wxMemoryDC dcBmp(bitmap);

    VAR_DEBUG(GetUpdateRegion());
    wxRegionIterator upd(GetUpdateRegion()); // get the update rect list
    while (upd)
    {
        int x = scroll.x + upd.GetX();
        int y = scroll.y + upd.GetY();
        int w = upd.GetW();
        int h = upd.GetH();
        VAR_DEBUG(x)(y)(w)(h);
        dc.Blit(x,y,w,h,&dcBmp,x,y,wxCOPY);
        upd++;
    }
}

//////////////////////////////////////////////////////////////////////////
// PROPAGATE UPDATES UPWARD
//////////////////////////////////////////////////////////////////////////

void Timeline::onViewUpdated( ViewUpdateEvent& event )
{
    SetVirtualSize(requiredWidth(),requiredHeight());
 //   getCursor().moveCursorOnUser(getCursor().getPosition()); // This is needed to reset iterators in model in case of clip addition/removal
    mRedrawOnIdle = true;
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

PlayerPtr Timeline::getPlayer() const
{
    return mPlayer;
}

VideoView& Timeline::getVideo()
{
    return *mVideoView;
}

const VideoView& Timeline::getVideo() const
{
    return *mVideoView;
}

AudioView& Timeline::getAudio()
{
    return *mAudioView;
}

const AudioView& Timeline::getAudio() const
{
    return *mAudioView;
}

wxPoint Timeline::getScrollOffset() const
{
    int scrollX, scrollY, ppuX, ppuY;
    GetViewStart(&scrollX,&scrollY);
    GetScrollPixelsPerUnit(&ppuX,&ppuY);
    return wxPoint(scrollX * ppuX, scrollY * ppuY);
}

pixel Timeline::requiredWidth() const
{
    return
        std::max(std::max(
        getWindow().GetClientSize().GetWidth(),                         // At least the widget size
        getZoom().timeToPixels(5 * model::Constants::sMinute)),                // Minimum width of 5 minutes
        getZoom().ptsToPixels(getSequence()->getNumberOfFrames()));     // At least enough to hold all clips
}

pixel Timeline::requiredHeight() const
{
    return
        std::max(
        getWindow().GetClientSize().GetHeight(),                        // At least the widget size
        Layout::sTimeScaleHeight +
        Layout::sMinimalGreyAboveVideoTracksHeight +
        getVideo().requiredHeight() +
        Layout::sAudioVideoDividerHeight +
        getAudio().requiredHeight() +
        Layout::sMinimalGreyBelowAudioTracksHeight);                 // Height of all combined components
}

//////////////////////////////////////////////////////////////////////////
// DRAW
//////////////////////////////////////////////////////////////////////////

void Timeline::draw(wxBitmap& bitmap) const
{
    LOG_DEBUG;
    wxMemoryDC dc(bitmap);

    // Get size of canvas
    int w = bitmap.GetWidth();
    int h = bitmap.GetHeight();

    // Set BG
    dc.SetPen(Layout::sBackgroundPen);
    dc.SetBrush(Layout::sBackgroundBrush);
    dc.DrawRectangle(0,0,w,h);

    // Draw timescale
    dc.SetBrush(wxNullBrush);
    dc.SetPen(Layout::sTimeScaleDividerPen);
    dc.DrawRectangle(0,0,w,Layout::sTimeScaleHeight);

    dc.SetFont(*Layout::sTimeScaleFont);

    // Draw seconds and minutes lines
    for (int ms = 0; getZoom().timeToPixels(ms) <= w; ms += model::Constants::sSecond)
    {
        int position = getZoom().timeToPixels(ms);
        bool isMinute = (ms % model::Constants::sMinute == 0);
        int height = Layout::sTimeScaleSecondHeight;

        if (isMinute)
        {
            height = Layout::sTimeScaleMinutesHeight;
        }

        dc.DrawLine(position,0,position,height);

        if (ms == 0)
        {
            dc.DrawText( "0", 5, Layout::sTimeScaleMinutesHeight );
        }
        else
        {
            if (isMinute)
            {
                wxDateTime t(ms / model::Constants::sHour, (ms % model::Constants::sHour) / model::Constants::sMinute, (ms % model::Constants::sMinute) / model::Constants::sSecond, ms % model::Constants::sSecond);
                wxString s = t.Format("%H:%M:%S.%l");
                wxSize ts = dc.GetTextExtent(s);
                dc.DrawText( s, position - ts.GetX() / 2, Layout::sTimeScaleMinutesHeight );
            }
        }
    }

    // Get video and audio bitmaps, possibly required for determining divider position
    const wxBitmap& videotracks = getVideo().getBitmap();
    const wxBitmap& audiotracks = getAudio().getBitmap();

    dc.DrawBitmap(videotracks,wxPoint(0,getDivider().getVideoPosition()));
    dc.DrawBitmap(audiotracks,wxPoint(0,getDivider().getAudioPosition()));

    getDivider().draw(dc);
    getIntervals().draw(dc);
    getDrag().draw(dc);
    getDrop().draw(dc);
    getCursor().draw(dc);
}

//////////////////////////////////////////////////////////////////////////
// CHANGE COMMANDS
//////////////////////////////////////////////////////////////////////////

void Timeline::Submit(command::RootCommand* c)
{
    model::Project::current()->Submit(c);
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void Timeline::serialize(Archive & ar, const unsigned int version)
{
    ar & *mZoom;
    ar & *mIntervals;
    ar & *mDivider;
}

template void Timeline::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void Timeline::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);

}} // namespace
