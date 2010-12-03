#ifndef SELECT_INTERVALS_H
#define SELECT_INTERVALS_H

#include <map>
#include <wx/dc.h>
#include <wx/event.h>
#include <boost/serialization/access.hpp>
#include <boost/serialization/version.hpp>
#include "Part.h"

namespace model {
class Clip;
typedef boost::shared_ptr<Clip> ClipPtr;
}

namespace gui { namespace timeline {

class EventTimelineCursorMoved;

class Intervals
    :   public wxEvtHandler // MUST BE FIRST INHERITED CLASS FOR WXWIDGETS EVENTS TO BE RECEIVED.
    ,   public Part
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION METHODS
    //////////////////////////////////////////////////////////////////////////

    Intervals(Timeline* timeline);
    virtual ~Intervals();

    //////////////////////////////////////////////////////////////////////////
    // MARKING / TOGGLING INTERFACE
    //////////////////////////////////////////////////////////////////////////
    
    bool isEmpty(); ///< @return true if there is at least one marked interval

    wxRegion get();
    void set(wxRegion region);

    void addBeginMarker();
    void addEndMarker();

    void startToggle();
    void endToggle();

    void update(long newCursorPosition); ///< To be called when the cursor is moved.
    void change(long begin, long end, bool add); ///< To be called for the undo/redo mechanism.
    void clear(); ///< Clear all marked intervals.

    //////////////////////////////////////////////////////////////////////////
    // ACTIONS ON THE MARKED AREAS
    //////////////////////////////////////////////////////////////////////////

    void deleteMarked();
    void deleteUnmarked();

    //////////////////////////////////////////////////////////////////////////
    // DRAWING
    //////////////////////////////////////////////////////////////////////////

    void draw(wxDC& dc) const;

private:

    wxRegion mMarkedIntervals;

    bool mNewIntervalActive;
    long mNewIntervalBegin;
    long mNewIntervalEnd;

    bool mToggleActive;
    long mToggleBegin;
    long mToggleEnd;

    //////////////////////////////////////////////////////////////////////////
    // EVENTS
    //////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    wxRect makeRect(long x1, long x2) const;
    wxRect ptsToPixels(wxRect rect) const;
    void refresh(long begin, long end);

    typedef std::map< model::ClipPtr, model::ClipPtr > ReplacementMap;
    ReplacementMap findReplacements(TrackView* track);

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
BOOST_CLASS_VERSION(gui::timeline::Intervals, 1)

#endif // SELECT_INTERVALS_H
