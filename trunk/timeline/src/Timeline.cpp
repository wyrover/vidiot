#include "Timeline.h"
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <wx/dcclient.h>
#include <algorithm>
#include <wx/dcmemory.h>
#include "Constants.h"
#include "Layout.h"
#include "UtilLog.h"
#include "Options.h"
#include "Player.h"
#include "Preview.h"
#include "Window.h"
#include "Intervals.h"
#include "Selection.h"
#include "MousePointer.h"
#include "Scrolling.h"
#include "Cursor.h"
#include "Drag.h"
#include "Divider.h"
#include "Tooltip.h"
#include "Menu.h"
#include "Project.h"
#include "Zoom.h"
#include "Sequence.h"
#include "SequenceView.h"
#include "State.h"
#include "ViewMap.h"
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
,   mPlayer(Window::get().getPreview().openTimeline(sequence,this))
//////////////////////////////////////////////////////////////////////////
,   mZoom(new Zoom(this))
,   mViewMap(new ViewMap(this))
,   mIntervals(new Intervals(this))
,   mMousePointer(new MousePointer(this))
,   mScroll(new Scrolling(this))
,   mSelection(new Selection(this))
,   mCursor(new Cursor(this))
,   mDrag(new Drag(this))
,   mTooltip(new Tooltip(this))
,   mDivider(new Divider(this))
,   mStateMachine(new state::Machine(*this))
,   mMenuHandler(new MenuHandler(this))
//////////////////////////////////////////////////////////////////////////
,   mSequenceView(new SequenceView(this))
//////////////////////////////////////////////////////////////////////////
{
    VAR_DEBUG(this);

    // To ensure that for newly opened timelines the initial position is ok
    // (should take 'minimum position' into account). This can only be done
    // after both mDivider AND mVideoView are initialized.
    getDivider().setPosition(getDivider().getPosition()); 

    init();

    Bind(wxEVT_PAINT,               &Timeline::onPaint,              this);
    Bind(wxEVT_ERASE_BACKGROUND,    &Timeline::onEraseBackground,    this);
    Bind(wxEVT_SIZE,                &Timeline::onSize,               this);
}

Timeline::~Timeline()
{
    VAR_DEBUG(this);

    deinit();

    Unbind(wxEVT_PAINT,               &Timeline::onPaint,              this);
    Unbind(wxEVT_ERASE_BACKGROUND,    &Timeline::onEraseBackground,    this);
    Unbind(wxEVT_SIZE,                &Timeline::onSize,               this);

    Window::get().getPreview().closeTimeline(this);

    delete mSequenceView;   mSequenceView = 0;

    delete mMenuHandler;    mMenuHandler = 0;
    delete mStateMachine;     mStateMachine = 0;
    delete mDivider;        mDivider = 0;
    delete mTooltip;        mTooltip = 0;
    delete mDrag;           mDrag = 0;
    delete mCursor;         mCursor = 0;
    delete mSelection;      mSelection = 0;
    delete mMousePointer;   mMousePointer = 0;
    delete mIntervals;      mIntervals = 0;
    delete mViewMap;        mViewMap = 0;
    delete mZoom;           mZoom = 0;
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

SequenceView& Timeline::getSequenceView()
{
    return *mSequenceView;
}

const SequenceView& Timeline::getSequenceView() const
{
    return *mSequenceView;
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

Scrolling& Timeline::getScrolling()
{ 
    return *mScroll; 
}

const Scrolling& Timeline::getScrolling() const
{ 
    return *mScroll; 
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

Tooltip& Timeline::getTooltip()
{ 
    return *mTooltip; 
}

const Tooltip& Timeline::getTooltip() const
{ 
    return *mTooltip; 
}

Divider& Timeline::getDivider()
{
    return *mDivider;
}

const Divider& Timeline::getDivider() const
{
    return *mDivider;
}

state::Machine& Timeline::getStateMachine()
{
    return *mStateMachine;
}

const state::Machine& Timeline::getStateMachine() const
{
    return *mStateMachine;
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

void Timeline::onSize(wxSizeEvent& event)
{
    getSequenceView().invalidateBitmap(); // Redraw the sequence
    resize();
}

void Timeline::onEraseBackground(wxEraseEvent& event)
{
    wxDC* dc = event.GetDC();
    dc->SetPen(Layout::sBackgroundPen);
    dc->SetBrush(Layout::sBackgroundBrush);
    dc->DrawRectangle(wxPoint(0,0),dc->GetSize());
    //event.Skip(); // The official way of doing it
}

void Timeline::onPaint( wxPaintEvent &WXUNUSED(event) )
{
    wxPaintDC dc( this );
    DoPrepareDC(dc); // Adjust for logical coordinates, not device coordinates

    wxPoint scroll = getScrolling().getOffset();

    wxBitmap bitmap = getSequenceView().getBitmap();
    wxMemoryDC dcBmp(bitmap);

    wxRegionIterator upd(GetUpdateRegion()); // get the update rect list
    while (upd)
    {
        int x = scroll.x + upd.GetX();
        int y = scroll.y + upd.GetY();
        int w = upd.GetW();
        int h = upd.GetH();
        dc.Blit(x,y,w,h,&dcBmp,x,y,wxCOPY);
        upd++;
    }
}

//////////////////////////////////////////////////////////////////////////
// PROPAGATE UPDATES UPWARD
//////////////////////////////////////////////////////////////////////////

void Timeline::onViewUpdated( ViewUpdateEvent& event )
{
    resize();
    event.Skip();
}

void Timeline::onZoomChanged( ZoomChangeEvent& event )
{
    resize();
    event.Skip();
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

PlayerPtr Timeline::getPlayer() const
{
    return mPlayer;
}

pixel Timeline::requiredWidth() const
{
    return
        std::max(
        getWindow().GetClientSize().GetWidth(),                         // At least the widget size
        getSequenceView().requiredWidth());                             // At least enough to hold all clips
}

pixel Timeline::requiredHeight() const
{
    return
        std::max(
        getWindow().GetClientSize().GetHeight(),                        // At least the widget size
        getSequenceView().requiredHeight());                            // At least enough to hold all tracks
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void Timeline::resize()
{
    SetVirtualSize(requiredWidth(),requiredHeight());
    Refresh(false);
    // NOT: Update(); RATIONALE: This will cause too much updates when 
    //                           adding/removing/changing/replacing clips
    //                           which causes flickering.
}

void Timeline::draw(wxBitmap& bitmap) const
{
    FATAL("THIS BITMAP IS UNUSED");
}

//////////////////////////////////////////////////////////////////////////
// CHANGE COMMANDS
//////////////////////////////////////////////////////////////////////////

void Timeline::Submit(::command::RootCommand* c)
{
    model::Project::get().Submit(c);
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
