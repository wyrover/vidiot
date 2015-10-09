// Copyright 2013-2015 Eric Raijmakers.
//
// This file is part of Vidiot.
//
// Vidiot is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Vidiot is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Vidiot. If not, see <http://www.gnu.org/licenses/>.

#include "Timeline.h"

#include "AudioView.h"
#include "Constants.h"
#include "Cursor.h"
#include "Details.h"
#include "Drag.h"
#include "Intervals.h"
#include "Keyboard.h"
#include "Menu.h"
#include "Mouse.h"
#include "Player.h"
#include "Preview.h"
#include "Scrolling.h"
#include "Selection.h"
#include "Sequence.h"
#include "SequenceView.h"
#include "State.h"
#include "ThumbnailView.h"
#include "TimelineClipboard.h"
#include "Tooltip.h"
#include "Track.h"
#include "Trim.h"
#include "UtilLog.h"
#include "UtilLogWxwidgets.h"
#include "VideoView.h"
#include "ViewMap.h"
#include "Window.h"
#include "Zoom.h"

namespace gui { namespace timeline {

const pixel Timeline::SnapDistance{ 50 };

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

Timeline::Timeline(wxWindow *parent, const model::SequencePtr& sequence, bool beginTransacted)
:   wxScrolledWindow(parent,wxID_ANY,wxPoint(0,0),wxDefaultSize,wxHSCROLL|wxVSCROLL)
//////////////////////////////////////////////////////////////////////////
,   mSequence(sequence)
,   mPlayer(Window::get().getPreview().openTimeline(sequence,this))
,   mTransaction(false)
,   mShift(0)
,   mBufferBitmap()
,   mExecuteOnIdle()
,   mRenderThumbnails(false)
,   mActive(false)
//////////////////////////////////////////////////////////////////////////
,   mTrim(new Trim(this))
,   mZoom(new Zoom(this))
,   mViewMap(new ViewMap(this))
,   mIntervals(new Intervals(this))
,   mKeyboard(new Keyboard(this))
,   mClipboard(new TimelineClipboard(this))
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

    SetBackgroundColour(wxColour{ 212, 208, 200 }); // Background colour
    SetBackgroundStyle(wxBG_STYLE_PAINT); // For the buffered DC in onPaint()

    wxSize size = GetClientSize();
    mBufferBitmap = (size.x > 0 && size.y > 0) ? boost::make_shared<wxBitmap>(size) : nullptr;

    mStateMachine->start();

    BindAndCatchExceptions(this, wxEVT_PAINT,               &Timeline::onPaint,              this);
    BindAndCatchExceptions(this, wxEVT_ERASE_BACKGROUND,    &Timeline::onEraseBackground,    this);
    BindAndCatchExceptions(this, wxEVT_SIZE,                &Timeline::onSize,               this);
    BindAndCatchExceptions(this, wxEVT_IDLE,                &Timeline::onIdle,               this);

    if (beginTransacted)
    {
        beginTransaction();
    }

    // Ensure that for newly opened timelines the initial position is ok
    getSequenceView().resetDividerPosition();

    mExecuteOnSize = [this]
    {
        // Delayed until after first resize event.
        // Otherwise GTK port will call 'alignCenterPts...' before the
        // widget's size is properly initialized.
        mExecuteOnIdle = std::bind(&Timeline::alignCenterPtsAfterInitialization, this); // Run this when the whole startup is done.
    };
}

Timeline::~Timeline()
{
    VAR_DEBUG(this);

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
	delete mScroll;			mScroll = 0;
    delete mMouse;          mMouse = 0;
    delete mIntervals;      mIntervals = 0;
    delete mClipboard;      mClipboard = 0;
    delete mKeyboard;       mKeyboard = 0;
    delete mViewMap;        mViewMap = 0;
    delete mZoom;           mZoom = 0;
    delete mTrim;           mTrim = 0;

    Window::get().getPreview().closeTimeline(this); // This closes the Player
    mPlayer = 0;
    mBufferBitmap = nullptr;
    mExecuteOnIdle = nullptr;

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

TimelineClipboard& Timeline::getClipboard()
{
    return *mClipboard;
}

const TimelineClipboard& Timeline::getClipboard() const
{
    return *mClipboard;
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

void Timeline::onIdle(wxIdleEvent& event)
{
    if (mExecuteOnIdle)
    {
        mExecuteOnIdle();
    }
    event.Skip();
}

void Timeline::onSize(wxSizeEvent& event)
{
    if (mExecuteOnSize)
    {
        mExecuteOnSize();
        mExecuteOnSize = nullptr;
    }
    wxSize size = GetClientSize();
    mBufferBitmap = (size.x > 0 && size.y > 0) ? boost::make_shared<wxBitmap>(size) : nullptr;
    resize();
    event.Skip();
}

void Timeline::onEraseBackground(wxEraseEvent& event)
{
    // NOT: event.Skip(); // The official way of doing it
}

//////////////////////////////////////////////////////////////////////////
// DRAWING
//////////////////////////////////////////////////////////////////////////

void Timeline::onPaint(wxPaintEvent &event)
{
    if (mTransaction)
    {
        return;
    }

    boost::scoped_ptr<wxDC> dc;
    if (!IsDoubleBuffered() &&
        mBufferBitmap != nullptr)
    {
        // A dedicated buffer bitmap is used. Without it I had conflicts between the buffered
        // bitmap used for VideoDisplay and Timeline: when pressing 'b' (trim begin) during
        // playback, one of the playback frames ended popping up over the timeline.
        dc.reset(new wxBufferedPaintDC(this, *mBufferBitmap, wxBUFFER_CLIENT_AREA/*wxBUFFER_VIRTUAL_AREA*/ ));  // See: http://trac.wxwidgets.org/ticket/15497
    }
    else
    {
        dc.reset(new wxPaintDC(this));
    }
    // NOT: DoPrepareDC(*dc); -- scrolling (and shifting) offsets are calculated manually (using this automated recalculation was too difficult)
    // NOT: dc->SetLogicalOrigin(-mShift,0);

    wxPoint scroll = getScrolling().getOffset();

    wxRegion updatedRegion(GetUpdateRegion());
    getSequenceView().draw(*dc, updatedRegion, scroll);
    getIntervals().draw(*dc, updatedRegion, scroll);
    getDrag().drawDraggedClips(*dc, updatedRegion, scroll); // Dragged clips are drawn 'under' the cursor and snaps
    getCursor().draw(*dc,updatedRegion, scroll);
    getDrag().drawSnaps(*dc, updatedRegion, scroll); // Snaps are drawn on top of the cursor.
    getTrim().drawSnaps(*dc, updatedRegion, scroll); // Snaps are drawn on top of the cursor.

    //dc->SetPen(wxPen(*wxRED, 4));
    //dc->SetBrush(*wxTRANSPARENT_BRUSH);
    //wxRegionIterator upd(updatedRegion); // get the update rect list
    //if (upd)
    //{
    //    while (upd)
    //    {
    //        dc->DrawRectangle(upd.GetRect());
    //        upd++;
    //    }
    //}
    //dc->SetPen(wxPen(*wxBLUE,4));
    //dc->SetBrush(*wxTRANSPARENT_BRUSH);
    //wxRegionIterator upd2(updatedRegion); // get the update rect list
    //if (upd2)
    //{
    //    while (upd2)
    //    {
    //        wxRect r(upd2.GetRect().GetPosition(), upd2.GetRect().GetSize());
    //        r.SetPosition(r.GetPosition() - scroll);
    //        dc->DrawRectangle(r);
    //        upd2++;
    //    }
    //}

}

void Timeline::drawLine(wxDC& dc, const wxRegion& region, const wxPoint& offset, pts position, const wxPen& pen) const
{
    pixel pos = getZoom().ptsToPixels(position);
    wxRect r(pos - offset.x,0,1,getSequenceView().getH());
    wxRegion overlap(region);
    overlap.Intersect(r);
    wxRegionIterator upd(overlap);
    if (upd)
    {
        dc.SetPen(pen);
        while (upd)
        {
            dc.DrawLine(upd.GetX(),0,upd.GetX(),getSequenceView().getH());
            upd++;
        }
    }
}

void Timeline::drawDivider(wxDC& dc, wxRegion region, const wxPoint& offset, wxPoint position, pixel height) const
{
    int timelineWidth{ GetClientSize().GetWidth() };

    // Region is in screen coordinates.
    // position and height are in logical coordinates (entire timeline)

    // Note: Not use simply DrawRectangle. Given the size of the drawing (width of entire timeline)
    //       DrawRectangle sometimes fails to draw (particularly when zooming in fully).
    dc.SetBrush(wxBrush{ wxColour{ 132, 132, 132 } });
    dc.SetPen(wxPen{ wxColour{ 132, 132, 132 }, 1 });
    dc.DrawRectangle(wxPoint(0, position.y - offset.y), wxSize(timelineWidth, height)); // Draw bg color

    dc.SetPen(wxPen{ wxColour{ 64, 64, 64 }, 1 });
    int top{ position.y - offset.y};
    int bottom{ top + height - 1};
    dc.DrawLine(wxPoint(0, top), wxPoint(timelineWidth, top));
    dc.DrawLine(wxPoint(0, bottom), wxPoint(timelineWidth, bottom));
    dc.DrawLine(wxPoint(0, position.y) - offset, wxPoint(0, position.y + height) - offset); // Left line of the bounding box
    dc.DrawLine(wxPoint(GetVirtualSize().GetWidth() - 1 - offset.x, top), wxPoint(GetVirtualSize().GetWidth() - 1 - offset.x, bottom)); // Right line of the bounding box
}

void Timeline::copyRect(wxDC& dc, wxRegion region, const wxPoint& offset, const wxBitmap& bitmap, const wxRect& roi, bool mask) const
{
    ASSERT_IMPLIES(mask, bitmap.GetMask() != 0);
    // Region is in screen coordinates.
    // roi is in logical coordinates (entire timeline)
    wxRect scrolledRoi(roi);
    scrolledRoi.x -= offset.x;
    scrolledRoi.y -= offset.y;
    region.Intersect(scrolledRoi);

    wxRegionIterator upd(region); // get the update rect list
    if (upd)
    {
        wxMemoryDC dcBmp;
        dcBmp.SelectObjectAsSource(bitmap);
        while (upd)
        {
            int x = upd.GetX();
            int y = upd.GetY();
            int w = upd.GetW();
            int h = upd.GetH();
            dc.Blit(x, y, w, h, &dcBmp, offset.x + x - roi.x, offset.y + y - roi.y, wxCOPY, mask);
            upd++;
        }
    }
};

void Timeline::clearRect(wxDC& dc, wxRegion region, const wxPoint& offset, const wxRect& cleared) const
{
    // Region is in screen coordinates.
    // cleared is in logical coordinates (entire timeline)
    wxRect r(cleared);
    r.x -= offset.x;
    r.y -= offset.y;
    region.Intersect(r);

    wxRegionIterator upd(region);
    if (upd)
    {
        dc.SetBrush(wxBrush{ wxColour{ 212, 208, 200 } }); // Background colour
        dc.SetPen(wxPen{ wxColour{ 212, 208, 200 } }); // Background colour
        while (upd)
        {
            dc.DrawRectangle(upd.GetRect());
            upd++;
        }
    }
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

void Timeline::activate(bool active)
{
    VAR_INFO(this)(active);
    mActive = active;
    getMenuHandler().activate(active);
    if (active)
    {
        Window::get().getPreview().selectTimeline(this);
        Window::get().getDetailsView().selectTimeline(this);
    }
}

bool Timeline::isActive() const
{
    return mActive;
}

Player* Timeline::getPlayer() const
{
    return mPlayer;
}

void Timeline::refreshPts(pts position)
{
    repaint(wxRect(getZoom().ptsToPixels(position),0,1,getSequenceView().getSize().GetHeight()));
}

void Timeline::repaint(wxRect rect)
{
    wxPoint scroll = getScrolling().getOffset();
    rect.x -= scroll.x;
    rect.y -= scroll.y;
    RefreshRect(rect, false);
}

void Timeline::setShift(pixel shift)
{
    if (mShift != shift)
    {
        mShift = shift;
        Refresh(false);
    }
}

pixel Timeline::getShift() const
{
    return mShift;
}

void Timeline::resize()
{
    getSequenceView().invalidateRect();

    wxSize clientSize{ GetClientSize() };
    wxSize requiredSize{ getSequenceView().getDefaultSize() };

    SetVirtualSize(wxSize{ std::max(clientSize.x, requiredSize.x), std::max(clientSize.y, requiredSize.y) });
}

bool Timeline::renderThumbnails() const
{
    return mRenderThumbnails;
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
    // See [#166]: First playback after trim (or shift-delete) shows only black video sometimes.
    // This was caused by the playback position not being reset (normally
    // done in AClipEdit::Do()/Undo() when changing the timeline. However,
    // In case of a 'transacted' edit (multiple edits to be 'viewed as one
    // edit', this was not done.
    modelChanged();
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
    getPlayer()->moveTo(getTimeline().getCursor().getLogicalPosition());
    getPlayer()->updateLength();
}

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION STEPS
//////////////////////////////////////////////////////////////////////////

void Timeline::alignCenterPtsAfterInitialization()
{
    // Give newly opened timelines the keyboard focus.
    SetFocus();

    getScrolling().alignCenterPts();
    getCursor().setLogicalPosition(getCursor().getLogicalPosition()); // Set to the proper position after loading
    mExecuteOnIdle = std::bind(&Timeline::readFocusedThumbnails, this);
}

void Timeline::readFocusedThumbnails()
{
    // After the scroll bar has been moved to its proper position, the then
    // focused thumbnails may be shown. If done sooner, then thumbnails
    // that are not shown initially will first be rendered. That results in
    // the thumbnails in the visible area to be drawn a bit later (lag).
    mRenderThumbnails = true;

    // Trigger paint event upon which the focused thumbnails are rendered.
    Refresh();

    // Upon idle all other thumbnails are rendered
    mExecuteOnIdle = std::bind(&Timeline::readInitialThumbnailsAfterInitialization, this);
}

void Timeline::readInitialThumbnailsAfterInitialization()
{
    for (ClipPreview* view : getViewMap().getClipPreviews())
    {
        view->scheduleInitialRendering();
    }
    mExecuteOnIdle = nullptr;
    ignoreIdleEvents();
}

void Timeline::ignoreIdleEvents()
{
    // First idle after startup indicates 'init done'
    Unbind(wxEVT_IDLE,                &Timeline::onIdle,               this);
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void Timeline::serialize(Archive & ar, const unsigned int version)
{
    try
    {
        ar & boost::serialization::make_nvp("zoom",*mZoom);
        ar & boost::serialization::make_nvp("intervals",*mIntervals);
        ar & boost::serialization::make_nvp("scrolling",*mScroll); // Must be done before cursor (since cursor uses the scroll position)
        ar & boost::serialization::make_nvp("cursor",*mCursor);
        if (version >= 2)
        {
            ar & boost::serialization::make_nvp("player",*mPlayer);
        }
    }
    catch (boost::archive::archive_exception& e) { VAR_ERROR(e.what());                         throw; }
    catch (boost::exception &e)                  { VAR_ERROR(boost::diagnostic_information(e)); throw; }
    catch (std::exception& e)                    { VAR_ERROR(e.what());                         throw; }
    catch (...)                                  { LOG_ERROR;                                   throw; }
}

template void Timeline::serialize<boost::archive::xml_oarchive>(boost::archive::xml_oarchive& ar, const unsigned int archiveVersion);
template void Timeline::serialize<boost::archive::xml_iarchive>(boost::archive::xml_iarchive& ar, const unsigned int archiveVersion);

}} // namespace
