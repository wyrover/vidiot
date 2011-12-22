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
#include "Tooltip.h"
#include "Menu.h"
#include "Project.h"
#include "Zoom.h"
#include "Sequence.h"
#include "SequenceView.h"
#include "State.h"
#include "Trim.h"
#include "ViewMap.h"
#include "ViewUpdateEvent.h"
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
,   mTrim(new Trim(this))
,   mStateMachine(new state::Machine(*this))
,   mMenuHandler(new MenuHandler(this))
//////////////////////////////////////////////////////////////////////////
,   mSequenceView(new SequenceView(this))
//////////////////////////////////////////////////////////////////////////
{
    VAR_DEBUG(this);

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

    delete mSequenceView;   mSequenceView = 0;

    delete mMenuHandler;    mMenuHandler = 0;
    delete mStateMachine;   mStateMachine = 0;
    delete mTrim;           mTrim = 0;
    delete mTooltip;        mTooltip = 0;
    delete mDrag;           mDrag = 0;
    delete mCursor;         mCursor = 0;
    delete mSelection;      mSelection = 0;
    delete mMousePointer;   mMousePointer = 0;
    delete mIntervals;      mIntervals = 0;
    delete mViewMap;        mViewMap = 0;
    delete mZoom;           mZoom = 0;

    Window::get().getPreview().closeTimeline(this); // This closes the Player
    mPlayer = 0;
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

Trim& Timeline::getTrim()
{
    return *mTrim;
}

const Trim& Timeline::getTrim() const
{
    return *mTrim;
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
    resize();
}

void Timeline::onEraseBackground(wxEraseEvent& event)
{
    // NOT: event.Skip(); // The official way of doing it
}

void Timeline::onPaint( wxPaintEvent &event )
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

    getDrag().draw(dc);
    getCursor().draw(dc);
}

//////////////////////////////////////////////////////////////////////////
// PROPAGATE UPDATES UPWARD
//////////////////////////////////////////////////////////////////////////

void Timeline::onViewUpdated( ViewUpdateEvent& event )
{
    Refresh(false);
    event.Skip();
}

void Timeline::onZoomChanged( ZoomChangeEvent& event )
{
    resize();
    event.Skip();
}

void Timeline::activate()
{
    Window::get().getPreview().selectTimeline(this);
    getMenuHandler().activate();
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

Player* Timeline::getPlayer() const
{
    return mPlayer;
}

pixel Timeline::requiredWidth() const
{
    FATAL;
    return 0;
}

pixel Timeline::requiredHeight() const
{
    FATAL;
    return 0;
}

void Timeline::refreshPts(pts position)
{
    pixel pixpos = getZoom().ptsToPixels(position) - getScrolling().getOffset().x;
    getTimeline().RefreshRect(wxRect(pixpos,0,1,getSequenceView().getHeight()), false);
}

void Timeline::refreshLines(pixel from, pixel length)
{
    getTimeline().RefreshRect(wxRect(0,from,getSequenceView().getWidth(),length), false);
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void Timeline::resize()
{
    SetVirtualSize(getSequenceView().getWidth(),getSequenceView().getHeight());
    Refresh();
    // NOT: Update(); RATIONALE: This will cause too much updates when
    //                           adding/removing/changing/replacing clips
    //                           which causes flickering.
}

void Timeline::draw(wxBitmap& bitmap) const
{
    FATAL;
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
}

template void Timeline::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void Timeline::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);

}} // namespace