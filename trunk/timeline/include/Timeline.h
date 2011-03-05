#ifndef TIMELINE_H
#define TIMELINE_H

#include <boost/serialization/access.hpp>
#include <boost/serialization/version.hpp>
#include "View.h"

namespace command {
    class RootCommand;
}

namespace gui { namespace timeline {

class Timeline
:   public wxScrolledWindow
,   public View
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION METHODS
    //////////////////////////////////////////////////////////////////////////

    Timeline(wxWindow *parent, model::SequencePtr sequence);
    virtual ~Timeline();

    //////////////////////////////////////////////////////////////////////////
    // PART
    //////////////////////////////////////////////////////////////////////////

    Timeline& getTimeline();
    const Timeline& getTimeline() const;
    SequenceView& getSequenceView();
    const SequenceView& getSequenceView() const;
    Zoom& getZoom();
    const Zoom& getZoom() const;
    ViewMap& getViewMap();
    const ViewMap& getViewMap() const;
    Intervals& getIntervals();
    const Intervals& getIntervals() const;
    MousePointer& getMousepointer();
    const MousePointer& getMousepointer() const;
    Scrolling& getScrolling();
    const Scrolling& getScrolling() const;
    Selection& getSelection();
    const Selection& getSelection() const;
    MenuHandler& getMenuHandler();
    const MenuHandler& getMenuHandler() const;
    Cursor& getCursor();
    const Cursor& getCursor() const;
    Drag& getDrag();
    const Drag& getDrag() const;
    Tooltip& getTooltip();
    const Tooltip& getTooltip() const;
    Drop& getDrop();
    const Drop& getDrop() const;
    Divider& getDivider();
    const Divider& getDivider() const;
    model::SequencePtr getSequence();
    const model::SequencePtr getSequence() const;

    //////////////////////////////////////////////////////////////////////////
    // GUI EVENTS
    //////////////////////////////////////////////////////////////////////////

    void onSize(wxSizeEvent& event);
    void onEraseBackground(wxEraseEvent& event);
    void onPaint( wxPaintEvent &event );

    //////////////////////////////////////////////////////////////////////////
    // EVENTS
    //////////////////////////////////////////////////////////////////////////

    void onViewUpdated( ViewUpdateEvent& event ); ///< @see View::onChildViewUpdated()
    void onZoomChanged( ZoomChangeEvent& event ); ///< @see View::onZoomChanged()

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    PlayerPtr getPlayer() const;

    pixel requiredWidth() const;  ///< @see View::requiredWidth()
    pixel requiredHeight() const; ///< @see View::requiredHeight()

    //////////////////////////////////////////////////////////////////////////
    // CHANGE COMMANDS
    //////////////////////////////////////////////////////////////////////////

    void Submit(::command::RootCommand* c);

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    model::SequencePtr mSequence;
    PlayerPtr mPlayer;

    //////////////////////////////////////////////////////////////////////////
    // PART -> Must be AFTER MEMBERS
    //////////////////////////////////////////////////////////////////////////

    Zoom* mZoom;
    ViewMap* mViewMap;
    Intervals* mIntervals;
    MousePointer* mMousePointer;
    Scrolling* mScroll;
    Selection* mSelection;
    Cursor* mCursor;  // Must be AFTER mPlayer
    Drag* mDrag;
    Tooltip* mTooltip;
    Drop* mDrop;
    Divider* mDivider;
    state::Machine* mMouseState; // Must be AFTER mViewMap due to constructor list.
    MenuHandler* mMenuHandler; // Init as last since it depends on other parts

    //////////////////////////////////////////////////////////////////////////
    // CHILDREN -> Must be AFTER PARTS
    //////////////////////////////////////////////////////////////////////////

    SequenceView* mSequenceView;

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    void resize();
    void draw(wxBitmap& bitmap) const; ///< @see View::draw()

    //////////////////////////////////////////////////////////////////////////
    // SERIALIZATION
    //////////////////////////////////////////////////////////////////////////

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);
};

}} // namespace

// Workaround needed to prevent compile-time errors (mpl_assertion_in_line...) with gcc
//#include  <boost/preprocessor/slot/counter.hpp>
//#include BOOST____PP_UPDATE_COUNTER()
//#line BOOST_____PP_COUNTER
BOOST_CLASS_VERSION(gui::timeline::Timeline, 1)

#endif // TIMELINE_H
