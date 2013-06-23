#include "Timeline.h"

#include "AudioView.h"
#include "Constants.h"
#include "Cursor.h"
#include "Details.h"
#include "Drag.h"
#include "Intervals.h"
#include "Keyboard.h"
#include "Layout.h"
#include "Menu.h"
#include "Mouse.h"
#include "Options.h"
#include "Player.h"
#include "Preview.h"
#include "Scrolling.h"
#include "Selection.h"
#include "Sequence.h"
#include "SequenceEvent.h"
#include "SequenceView.h"
#include "State.h"
#include "Tooltip.h"
#include "Track.h"
#include "Trim.h"
#include "UtilLog.h"
#include "UtilLogWxwidgets.h"
#include "VideoView.h"
#include "ViewMap.h"
#include "ViewUpdateEvent.h"
#include "Window.h"
#include "Zoom.h"

namespace gui { namespace timeline {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

Timeline::Timeline(wxWindow *parent, model::SequencePtr sequence, bool beginTransacted)
:   wxScrolledWindow(parent,wxID_ANY,wxPoint(0,0),wxDefaultSize,wxHSCROLL|wxVSCROLL)
,   View(this) // Has itself as parent...
//////////////////////////////////////////////////////////////////////////
,   mSequence(sequence)
,   mPlayer(Window::get().getPreview().openTimeline(sequence,this))
,   mTransaction(false)
,   mShift(0)
//////////////////////////////////////////////////////////////////////////
,   mTrim(new Trim(this))
,   mZoom(new Zoom(this))
,   mViewMap(new ViewMap(this))
,   mIntervals(new Intervals(this))
,   mKeyboard(new Keyboard(this))
,   mMouse(new Mouse(this))
,   mScroll(new Scrolling(this))
,   mSelection(new Selection(this))
,   mCursor(new Cursor(this))
,   mDrag(new Drag(this))
,   mTooltip(new Tooltip(this))
,   mStateMachine(new state::Machine(*this))
,   mMenuHandler(new MenuHandler(this))
,   mDetails(Window::get().getDetailsView().openTimeline(this))
//////////////////////////////////////////////////////////////////////////
,   mSequenceView(new SequenceView(this))
//////////////////////////////////////////////////////////////////////////
{
    VAR_DEBUG(this);

    SetBackgroundColour(Layout::get().BackgroundColour);

    init();

    Bind(wxEVT_PAINT,               &Timeline::onPaint,              this);
    Bind(wxEVT_ERASE_BACKGROUND,    &Timeline::onEraseBackground,    this);
    Bind(wxEVT_SIZE,                &Timeline::onSize,               this);

    if (beginTransacted)
    {
        beginTransaction();
    }

    // Ensure that for newly opened timelines the initial position is ok
    getSequenceView().resetDividerPosition();
}

Timeline::~Timeline()
{
    VAR_DEBUG(this);

    deinit();

    Unbind(wxEVT_PAINT,               &Timeline::onPaint,              this);
    Unbind(wxEVT_ERASE_BACKGROUND,    &Timeline::onEraseBackground,    this);
    Unbind(wxEVT_SIZE,                &Timeline::onSize,               this);

    delete mSequenceView;   mSequenceView = 0;
    Window::get().getDetailsView().closeTimeline(this);

    delete mMenuHandler;    mMenuHandler = 0;
    delete mStateMachine;   mStateMachine = 0;
    delete mTooltip;        mTooltip = 0;
    delete mDrag;           mDrag = 0;
    delete mCursor;         mCursor = 0;
    delete mSelection;      mSelection = 0;
    delete mMouse;   mMouse = 0;
    delete mIntervals;      mIntervals = 0;
    delete mKeyboard;       mKeyboard = 0;
    delete mViewMap;        mViewMap = 0;
    delete mZoom;           mZoom = 0;
    delete mTrim;           mTrim = 0;

    Window::get().getPreview().closeTimeline(this); // This closes the Player
    mPlayer = 0;

    mSequence->clean();
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

Keyboard& Timeline::getKeyboard()
{
    return *mKeyboard;
}

const Keyboard& Timeline::getKeyboard() const
{
    return *mKeyboard;
}

Mouse& Timeline::getMouse()
{
    return *mMouse;
}

const Mouse& Timeline::getMouse() const
{
    return *mMouse;
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

Details& Timeline::getDetails()
{
    return *mDetails;
}

const Details& Timeline::getDetails() const
{
    return *mDetails;
}

//////////////////////////////////////////////////////////////////////////
// WX EVENTS
//////////////////////////////////////////////////////////////////////////

void Timeline::onSize(wxSizeEvent& event)
{
    getSequenceView().canvasResized(); // Required to give the sequenceview the correct original height; otherwise it's initially too small (causing white areas below the actual used part)
    resize();

    event.Skip();
}

void Timeline::onEraseBackground(wxEraseEvent& event)
{
    // NOT: event.Skip(); // The official way of doing it
}

void Timeline::onPaint( wxPaintEvent &event )
{
    wxPaintDC dc( this );
    DoPrepareDC(dc); // Adjust for logical coordinates, not device coordinates

    if (mTransaction)
    {
        return;
    }

    wxPoint scroll = getScrolling().getOffset();

    wxBitmap bitmap = getSequenceView().getBitmap();
    //if (getSequence()->isFrozen())
    //{
    //    bitmap = bitmap.ConvertToDisabled(64);
    //}

    wxMemoryDC dcBmp(bitmap);
    dc.SetLogicalOrigin(-mShift,0);

    if (mShift > 0)
    {
        dc.SetPen(Layout::get().BackgroundPen);
        dc.SetBrush(Layout::get().BackgroundBrush);
        dc.DrawRectangle(-mShift,0,mShift,dc.GetSize().GetHeight());
    }

    wxRegionIterator upd(GetUpdateRegion()); // get the update rect list
    while (upd)
    {
        int x = scroll.x + upd.GetX() - mShift;
        int y = scroll.y + upd.GetY();
        int w = upd.GetW();
        int h = upd.GetH();
        dc.Blit(x,y,w,h,&dcBmp,x,y,wxCOPY);
        upd++;
    }

    //if (getSequence()->isFrozen())
    //{
    //    wxString progressText(_("Render in progress"));
    //    dc.SetTextForeground(Layout::get().TimelineRenderInProgressColour);
    //    dc.SetFont(Layout::get().RenderInProgressFont);
    //    wxSize textSize = dc.GetTextExtent(progressText);
    //    dc.DrawText(progressText, (GetSize().GetWidth() - textSize.GetWidth()) / 2, (bitmap.GetHeight() - textSize.GetHeight()) / 2);
    //}

    getDrag().draw(dc);
    getTrim().draw(dc);
    getCursor().draw(dc);
}

//////////////////////////////////////////////////////////////////////////
// PROPAGATE UPDATES UPWARD
//////////////////////////////////////////////////////////////////////////

void Timeline::onViewUpdated( ViewUpdateEvent& event )
{
    // NOT: resize(); // Adding this will cause lots of unneeded 'getSize()' calls
    Refresh(false);
    event.Skip();
}

void Timeline::onZoomChanged( ZoomChangeEvent& event )
{
    resize();
    event.Skip();
}

void Timeline::activate(bool active)
{
    VAR_INFO(this)(active);
    getMenuHandler().activate(active);
    if (active)
    {
        Window::get().getPreview().selectTimeline(this);
        Window::get().getDetailsView().selectTimeline(this);
    }
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

Player* Timeline::getPlayer() const
{
    return mPlayer;
}

wxSize Timeline::requiredSize() const
{
    FATAL;
    return wxSize(0,0);
}

void Timeline::refreshPts(pts position)
{
    pixel pixpos = getZoom().ptsToPixels(position) - getScrolling().getOffset().x;
    RefreshRect(wxRect(pixpos,0,1,getSequenceView().getSize().GetHeight()), false);
}

void Timeline::refreshLines(pixel from, pixel length)
{
    RefreshRect(wxRect(0,from,getSequenceView().getSize().GetWidth(),length), false);
}

void Timeline::setShift(pixel shift)
{
    mShift = shift;
    Refresh(false);
}

void Timeline::resize()
{
    wxSize oldSize = GetVirtualSize();
    getSequenceView().invalidateBitmap(); // Otherwise, the call to getSize() below will just return the current size...
    wxSize newSize = getSequenceView().getSize();
    if (oldSize != newSize)
    {
        SetVirtualSize(newSize);
    }
    // NOT: Update(); RATIONALE: This will cause too much updates when
    //                           adding/removing/changing/replacing clips
    //                           which causes flickering.
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void Timeline::draw(wxBitmap& bitmap) const
{
    FATAL;
}

//////////////////////////////////////////////////////////////////////////
// TRANSACTION
//////////////////////////////////////////////////////////////////////////

void Timeline::beginTransaction()
{
    mTransaction = true;
}

void Timeline::endTransaction()
{
    mTransaction = false;
}

//////////////////////////////////////////////////////////////////////////
// CHANGE COMMANDS
//////////////////////////////////////////////////////////////////////////

void Timeline::modelChanged()
{
    if (mTransaction) return;
    // This is required to
    // - reset model::Track iterators
    // - start at the last played position (and not start at the "buffered" position)
    getTimeline().getCursor().moveCursorOnUser(getTimeline().getCursor().getPosition());
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