#ifndef TIMELINE_H
#define TIMELINE_H

#include <boost/serialization/access.hpp>
#include <boost/serialization/version.hpp>
#include "Part.h"

namespace gui { namespace timeline {
    class ViewUpdateEvent;

class Timeline
:   public wxScrolledWindow
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
    Zoom& getZoom();
    const Zoom& getZoom() const;
    ViewMap& getViewMap();
    Intervals& getIntervals();
    MousePointer& getMousepointer();
    Selection& getSelection();
    MenuHandler& getMenuHandler();
    Cursor& getCursor();
    Drag& getDrag();
    Drop& getDrop();
    TimelineView& getView();
    model::SequencePtr getSequence();

    //////////////////////////////////////////////////////////////////////////
    // GUI EVENTS
    //////////////////////////////////////////////////////////////////////////

    void onIdle(wxIdleEvent& event);
    void onSize(wxSizeEvent& event);
    void onEraseBackground(wxEraseEvent& event);
    void onPaint( wxPaintEvent &event );

    //////////////////////////////////////////////////////////////////////////
    // PROPAGATE UPDATES UPWARD
    //////////////////////////////////////////////////////////////////////////

    void onViewUpdated( ViewUpdateEvent& event ); ///< @see View::onViewUpdated()

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    PlayerPtr getPlayer() const;
    wxPoint getScrollOffset() const;

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    model::SequencePtr mSequence;
    PlayerPtr mPlayer;
    bool mRedrawOnIdle;

    //////////////////////////////////////////////////////////////////////////
    // PART
    //////////////////////////////////////////////////////////////////////////

    Zoom* mZoom;
    ViewMap* mViewMap;
    Intervals* mIntervals;
    MousePointer* mMousePointer;
    Selection* mSelection;
    Cursor* mCursor;  // Must be AFTER mPlayer
    Drag* mDrag;
    Drop* mDrop;
    TimelineView* mTimelineView;
    state::Machine* mMouseState; // Must be AFTER mViewMap due to constructor list.
    MenuHandler* mMenuHandler; // Init as last since it depends on other parts

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
