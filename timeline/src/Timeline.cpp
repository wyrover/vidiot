#include "Timeline.h"
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <wx/dcclient.h>
#include <wx/dcmemory.h>
#include "Constants.h"
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
#include "Menu.h"
#include "Zoom.h"
#include "State.h"
#include "ViewMap.h"
#include "Drop.h"
#include "TimelineView.h"
#include "ViewMap.h"

namespace gui { namespace timeline {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

Timeline::Timeline(wxWindow *parent, model::SequencePtr sequence)
:   wxScrolledWindow(parent,wxID_ANY,wxPoint(0,0),wxDefaultSize,wxHSCROLL|wxVSCROLL|wxSUNKEN_BORDER)
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
,   mTimelineView(new TimelineView(this))
,   mMouseState(new state::Machine(*this))
,   mMenuHandler(new MenuHandler(this))
{
    LOG_INFO;

    SetScrollRate( 10, 10 );
    EnableScrolling(true,true);

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

Intervals& Timeline::getIntervals()
{ 
    return *mIntervals; 
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

TimelineView& Timeline::getView()
{
    return *mTimelineView;
}

model::SequencePtr Timeline::getSequence()
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
    SetVirtualSize(getView().requiredWidth(),getView().requiredHeight());
    mRedrawOnIdle = true;
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

    wxBitmap bitmap = getView().getBitmap();
    wxMemoryDC dcBmp(bitmap);

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
    getCursor().moveCursorOnUser(getCursor().getPosition()); // This is needed to reset iterators in model in case of clip addition/removal
    mRedrawOnIdle = true;
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

PlayerPtr Timeline::getPlayer() const
{
    return mPlayer;
}

wxPoint Timeline::getScrollOffset() const
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
void Timeline::serialize(Archive & ar, const unsigned int version)
{
    ar & *mZoom;
    ar & *mTimelineView;
    ar & *mIntervals;
}

template void Timeline::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void Timeline::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);

}} // namespace
