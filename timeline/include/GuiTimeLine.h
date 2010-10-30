#ifndef GUI_TIME_LINE_H
#define GUI_TIME_LINE_H

#include <wx/scrolwin.h>
#include <wx/dnd.h>
#include <wx/menu.h>
#include <boost/serialization/access.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/version.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/optional.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/enable_shared_from_this.hpp>
#include "GuiDataObject.h"
#include "ModelPtr.h"
#include "GuiPtr.h"
#include "State.h"
#include "SelectIntervals.h"
#include "UtilEvent.h"
#include "GuiTimeLineZoom.h"
#include "ViewMap.h"

namespace gui { namespace timeline {

class TrackUpdateEvent;

DECLARE_EVENT(TIMELINE_CURSOR_MOVED, EventTimelineCursorMoved, long);

class GuiTimeLine
:   public wxScrolledWindow
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION METHODS
    //////////////////////////////////////////////////////////////////////////

    /** The '0' pointer is used for recovery. */
    GuiTimeLine(model::SequencePtr sequence = model::SequencePtr());

    /**
     * Two step construction. First the constructor (in combination with serialize)
     * sets all relevant  members. Second, this method initializes all GUI stuff
     * including the bitmap.
     */
    void init(wxWindow *parent);

    virtual ~GuiTimeLine();

    //////////////////////////////////////////////////////////////////////////
    // SEQUENCE MENU
    //////////////////////////////////////////////////////////////////////////

    void OnAddVideoTrack(wxCommandEvent& WXUNUSED(event));
    void OnAddAudioTrack(wxCommandEvent& WXUNUSED(event));
    //void OnCloseSequence(wxCommandEvent& WXUNUSED(event));

    wxMenu* getMenu();

    //////////////////////////////////////////////////////////////////////////
    // MODEL EVENTS
    //////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////
    // GUI EVENTS
    //////////////////////////////////////////////////////////////////////////

    void OnSize(wxSizeEvent& event);
    void OnEraseBackground(wxEraseEvent& event);
    void OnPaint( wxPaintEvent &event );

    //////////////////////////////////////////////////////////////////////////
    // DRAWING EVENTS
    //////////////////////////////////////////////////////////////////////////

    void OnTrackUpdated( TrackUpdateEvent& event );

    wxBitmap getDragBitmap(wxPoint& hostspot);// const;
    void updateBitmap();

    void showDropArea(wxRect area);

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    model::SequencePtr getSequence() const;
    int getWidth() const;
    /**
     * @return index of given track. Audio tracks have a negative number.
     */
    int getIndex(GuiTimeLineTrackPtr track) const;

    wxPoint getScrollOffset() const;


    //////////////////////////////////////////////////////////////////////////
    // CURSOR
    //////////////////////////////////////////////////////////////////////////

    void moveCursorOnUser(int position);
    void setCursorPosition(long position);
    void moveCursorOnPlayback(long pts);

    //////////////////////////////////////////////////////////////////////////
    // FROM COORDINATES TO OBJECTS
    //////////////////////////////////////////////////////////////////////////

    /**
     * @param p position in virtual coordinates (thus, on the bitmap, not on the client area)
     * @return found clip and its leftmost position within the track
     * @return null pointer and 0 if not found
     */
    boost::tuple<GuiTimeLineClipPtr, int> findClip(wxPoint p) const;

    /**
     * @param yposition y position in virtual coordinates (thus, on the bitmap, not on the client area)
     * @return found track and its top position within the timeline
     * @return null pointer and 0 if not found
     */
    boost::tuple<GuiTimeLineTrackPtr,int> findTrack(int yposition) const;

    /**
     * Return list of all the tracks
     */
    GuiTimeLineTracks getTracks() const;

    /**
     * Returns a list containing ALL the clips currently in this timeline.
     */
    GuiTimeLineClips getClips() const;

private:

    friend class state::Idle;
    friend class state::Playing;
    friend class SelectIntervals;
    friend class state::MovingCursor;

    GuiTimeLineZoom mZoom;
    
    ViewMap mViewMap;

    TrackMap mTrackMap;
    ClipMap mClipMap;

    PlayerPtr mPlayer;
    wxBitmap mBitmap;
    long mCursorPosition;
    double mPlaybackTime;
    long mWidth;
    long mHeight;
    wxRect mDropArea;
    wxMenu mMenu;

    //////////////////////////////////////////////////////////////////////////
    // Interval selection
    //////////////////////////////////////////////////////////////////////////

    IntervalsPtr mSelectedIntervals;

    //////////////////////////////////////////////////////////////////////////
    //
    //////////////////////////////////////////////////////////////////////////

    /** Y-position of audio-video divider */
    int mDividerPosition;

    state::Machine mMouseState;

    model::SequencePtr mSequence;

    GuiTimeLineTracks mVideoTracks;
    GuiTimeLineTracks mAudioTracks;

    wxPoint mOrigin;

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    void updateSize();
    void DetermineWidth();
    void DetermineHeight();

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
BOOST_CLASS_VERSION(gui::timeline::GuiTimeLine, 1)
BOOST_CLASS_EXPORT(gui::timeline::GuiTimeLine)

#endif // GUI_TIME_LINE_H
