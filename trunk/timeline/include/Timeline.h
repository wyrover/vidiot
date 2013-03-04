#ifndef TIMELINE_H
#define TIMELINE_H

#include "View.h"

namespace command {
    class RootCommand;
}

namespace model {
    class EventSequenceFrozen;
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
    Trim& getTrim();
    const Trim& getTrim() const;
    state::Machine& getStateMachine();
    const state::Machine& getStateMachine() const;
    model::SequencePtr getSequence();
    const model::SequencePtr getSequence() const;
    Details& getDetails();
    const Details& getDetails() const;

    //////////////////////////////////////////////////////////////////////////
    // MODEL EVENTS
    //////////////////////////////////////////////////////////////////////////

    void onSequenceFrozen( model::EventSequenceFrozen& event );

    //////////////////////////////////////////////////////////////////////////
    // WX EVENTS
    //////////////////////////////////////////////////////////////////////////

    void onSize(wxSizeEvent& event);
    void onEraseBackground(wxEraseEvent& event);
    void onPaint( wxPaintEvent &event );

    //////////////////////////////////////////////////////////////////////////
    // TIMELINE EVENTS
    //////////////////////////////////////////////////////////////////////////

    void onViewUpdated( ViewUpdateEvent& event ); ///< @see View::onChildViewUpdated()
    void onZoomChanged( ZoomChangeEvent& event ); ///< @see View::onZoomChanged()

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    void activate(bool active); ///< To be called when the timeline becomes the active one

    Player* getPlayer() const;

    wxSize requiredSize() const;  ///< \see View::requiredSize()

    /// Refresh the rectangle for which
    /// [x,y,w,h] == [position,0,1,height],
    /// using logical coordinates and
    /// taking into account scrolling and zooming
    void refreshPts(pts position);

    /// Refresh several lines in the timeline
    void refreshLines(pixel from, pixel length);

    void setShift(pixel shift);

    //////////////////////////////////////////////////////////////////////////
    // TRANSACTION
    //////////////////////////////////////////////////////////////////////////

    /// Start a 'transaction'. All operations within the transaction do not lead
    /// to screen updates. When the complete set of operations is done (typically,
    /// after a editing operation is done completely), call endTransaction() to
    /// enable screen updates again. This is done to avoid useless intermediate
    /// updates during clip edits (only the result after the final modification
    /// is interesting).
    void beginTransaction();

    /// End a 'transaction'. This enables screen updates again.
    /// Typically, call modelChanged after doing this.
    void endTransaction();

    //////////////////////////////////////////////////////////////////////////
    // CHANGE COMMANDS
    //////////////////////////////////////////////////////////////////////////

    /// Reset the mouse pointer, triggering an update of both the model (since
    /// iterators are reset via the moveTo method) and the timeline (since
    /// resetting the cursor causes an update).
    void modelChanged();

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    model::SequencePtr mSequence;
    Player* mPlayer;

    bool mTransaction;

    pixel mShift;

    //////////////////////////////////////////////////////////////////////////
    // PART -> Must be AFTER MEMBERS
    //////////////////////////////////////////////////////////////////////////

    Trim* mTrim;
    Zoom* mZoom;
    ViewMap* mViewMap;
    Intervals* mIntervals;
    MousePointer* mMousePointer;
    Scrolling* mScroll;
    Selection* mSelection;
    Cursor* mCursor;  // Must be AFTER mPlayer
    Drag* mDrag;
    Tooltip* mTooltip;
    state::Machine* mStateMachine; // Must be AFTER mViewMap due to constructor list.
    MenuHandler* mMenuHandler; // Init as last since it depends on other parts
    Details* mDetails; // Must be after all other Parts (since it uses these during the initialization)

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