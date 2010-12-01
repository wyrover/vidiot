#ifndef TIMELINE_H
#define TIMELINE_H

#include <boost/serialization/access.hpp>
#include <boost/serialization/version.hpp>
#include "View.h"

namespace gui { namespace timeline {
    class ViewUpdateEvent;

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

    void onChildViewUpdated( ViewUpdateEvent& event ); ///< @see View::onChildViewUpdated()

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    VideoView& getVideo();
    AudioView& getAudio();
    PlayerPtr getPlayer() const;
    wxPoint getScrollOffset() const;
    int getDividerPosition() const;

    int requiredWidth();  ///< @see View::requiredWidth()
    int requiredHeight(); ///< @see View::requiredHeight()

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    model::SequencePtr mSequence;
    PlayerPtr mPlayer;
    bool mRedrawOnIdle;
    int mDividerPosition; ///< Y-position of audio-video divider

    //////////////////////////////////////////////////////////////////////////
    // PART -> Must be AFTER MEMBERS
    //////////////////////////////////////////////////////////////////////////

    Zoom* mZoom;
    ViewMap* mViewMap;
    Intervals* mIntervals;
    MousePointer* mMousePointer;
    Selection* mSelection;
    Cursor* mCursor;  // Must be AFTER mPlayer
    Drag* mDrag;
    Drop* mDrop;
    state::Machine* mMouseState; // Must be AFTER mViewMap due to constructor list.
    MenuHandler* mMenuHandler; // Init as last since it depends on other parts

    //////////////////////////////////////////////////////////////////////////
    // CHILDREN -> Must be AFTER PARTS
    //////////////////////////////////////////////////////////////////////////

    VideoView* mVideoView;
    AudioView* mAudioView;

    void draw(wxBitmap& bitmap); ///< @see View::draw()

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
