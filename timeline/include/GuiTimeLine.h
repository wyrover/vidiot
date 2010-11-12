#ifndef GUI_TIME_LINE_H
#define GUI_TIME_LINE_H

#include <wx/scrolwin.h>
#include <wx/dnd.h>
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
#include "UtilEnum.h"
#include "Intervals.h"
#include "Selection.h"
#include "MousePointer.h"
#include "UtilEvent.h"
#include "Cursor.h"
#include "Drag.h"
#include "Menu.h"
#include "Zoom.h"
#include "ViewMap.h"

namespace gui { namespace timeline {

class TrackUpdateEvent;
class Drag;

DECLAREENUM(MouseOnClipPosition, \
            ClipBetween, \
            ClipBegin, \
            ClipInterior, \
            ClipEnd);

struct PointerPositionInfo
{
    //////////////////////////////////////////////////////////////////////////
    // TRACK
    //////////////////////////////////////////////////////////////////////////

    /** Current track under the mouse pointer. 0 if none. */
    model::TrackPtr track;

    /** Y position of current track. 0 if no current track. */
    int trackPosition;

    //////////////////////////////////////////////////////////////////////////
    // CLIP
    //////////////////////////////////////////////////////////////////////////

    /** Current clip under the mouse pointer. 0 if none. */
    model::ClipPtr clip;

    MouseOnClipPosition logicalclipposition;

};

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
    // PARTS OVER WHICH THE IMPLEMENTATION IS SPLIT
    //////////////////////////////////////////////////////////////////////////

    Zoom& getZoom() { return mZoom; }
    const Zoom& getZoom() const { return mZoom; }
    ViewMap& getViewMap() { return mViewMap; }
    Intervals& getIntervals() { return mIntervals; }
    MousePointer& getMousepointer() { return mMousePointer; }
    Selection& getSelection() { return mSelection; }
    MenuHandler& getMenuHandler() { return mMenuHandler; }
    Cursor& getCursor() { return mCursor; }
    Drag& getDrag() { return mDrag; }

    //////////////////////////////////////////////////////////////////////////
    //
    //////////////////////////////////////////////////////////////////////////

    PlayerPtr getPlayer() const;

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

    void updateBitmap();

    void showDropArea(wxRect area);

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    model::SequencePtr getSequence() const;
    int getWidth() const;
    int getHeight() const;
    int getDividerPosition() const;



    wxPoint getScrollOffset() const;

    //////////////////////////////////////////////////////////////////////////
    // FROM COORDINATES TO OBJECTS
    //////////////////////////////////////////////////////////////////////////

    /**
     * @param p position in virtual coordinates (thus, on the bitmap, not on the client area)
     * @return found clip
     * @return null pointer if not found
     */
    GuiTimeLineClip* findClip(wxPoint p);

    /**
     * @param yposition y position in virtual coordinates (thus, on the bitmap, not on the client area)
     * @return found track and its top position within the timeline
     * @return null pointer and 0 if not found
     */
    boost::tuple<model::TrackPtr,int> findTrack(int yposition);

    PointerPositionInfo getPointerInfo(wxPoint pointerposition);

private:

    //////////////////////////////////////////////////////////////////////////
    // PARTS OVER WHICH THE IMPLEMENTATION IS SPLIT
    //////////////////////////////////////////////////////////////////////////

    Zoom mZoom;
    ViewMap mViewMap;
    Intervals mIntervals;
    MousePointer mMousePointer;
    Selection mSelection;
    MenuHandler mMenuHandler;
    Cursor mCursor;
    Drag mDrag;

    PlayerPtr mPlayer;
    wxBitmap mBitmap;
    double mPlaybackTime;
    long mWidth;
    long mHeight;
    wxRect mDropArea;

    //////////////////////////////////////////////////////////////////////////
    //
    //////////////////////////////////////////////////////////////////////////

    /** Y-position of audio-video divider */
    int mDividerPosition;

    /** Must be AFTER mViewMap due to constructor list. */
    state::Machine mMouseState;

    model::SequencePtr mSequence;

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
