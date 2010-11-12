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

    Zoom& getZoom();
    const Zoom& getZoom() const;
    ViewMap& getViewMap();
    Intervals& getIntervals();
    MousePointer& getMousepointer();
    Selection& getSelection();
    MenuHandler& getMenuHandler();
    Cursor& getCursor();
    Drag& getDrag();

    //////////////////////////////////////////////////////////////////////////
    // EVENTS
    //////////////////////////////////////////////////////////////////////////

    void onSize(wxSizeEvent& event);
    void onEraseBackground(wxEraseEvent& event);
    void onPaint( wxPaintEvent &event );
    void onTrackUpdated( TrackUpdateEvent& event );

    //////////////////////////////////////////////////////////////////////////

    void updateBitmap();

    void showDropArea(wxRect area);

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    PlayerPtr getPlayer() const;
    model::SequencePtr getSequence() const;
    int getWidth() const;
    int getHeight() const;
    int getDividerPosition() const;
    wxPoint getScrollOffset() const;

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
    state::Machine mMouseState; /** Must be AFTER mViewMap due to constructor list. */

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    PlayerPtr mPlayer;
    model::SequencePtr mSequence;
    wxBitmap mBitmap;
    long mWidth;
    long mHeight;
    wxRect mDropArea;
    /** Y-position of audio-video divider */
    int mDividerPosition;

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    void updateSize();
    void determineWidth();
    void determineHeight();

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
