#ifndef SELECT_INTERVALS_H
#define SELECT_INTERVALS_H

#include <map>
#include <wx/dc.h>
#include <boost/icl/interval_set.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/version.hpp>
#include "Part.h"
#include "UtilInt.h"

namespace model {
class IClip;
typedef boost::shared_ptr<IClip> IClipPtr;
}

namespace gui { namespace timeline {

class EventTimelineCursorMoved;

class Intervals
    :   public Part
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

    PtsIntervals get();
    void set(PtsIntervals region);
    void removeAll();

    void addBeginMarker();
    void addEndMarker();

    void startToggle();
    void endToggle();
    bool toggleIsAddition() const;

    void update(pixel newCursorPosition); ///< To be called when the cursor is moved.
    void change(PtsInterval interval, bool add); ///< To be called for the undo/redo mechanism.
    void clear(); ///< Clear all marked intervals.

    void refresh(); ///< Trigger a refresh

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

    PtsIntervals mMarkedIntervals;

    bool mNewIntervalActive;
    pts mNewIntervalBegin;
    pts mNewIntervalEnd;

    bool mToggleActive;
    pts mToggleBegin;
    pts mToggleEnd;

    //////////////////////////////////////////////////////////////////////////
    // EVENTS
    //////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    PtsInterval makeInterval(pts a, pts b) const;
    wxRect makeRect(PtsInterval interval) const;
    PixelInterval ptsToPixels(PtsInterval interval) const;
    void refreshInterval(PtsInterval interval);

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