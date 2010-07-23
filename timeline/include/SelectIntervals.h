#ifndef SELECT_INTERVALS_H
#define SELECT_INTERVALS_H

#include <wx/dc.h>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/version.hpp>
#include "GuiPtr.h"

namespace gui { namespace timeline {

class EventTimelineCursorMoved;

class SelectIntervals
    :   public boost::enable_shared_from_this<SelectIntervals>
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION METHODS
    //////////////////////////////////////////////////////////////////////////

    SelectIntervals();
    void init(GuiTimeLinePtr timeline);
    virtual ~SelectIntervals();

    //////////////////////////////////////////////////////////////////////////
    // MARKING / TOGGLING INTERFACE
    //////////////////////////////////////////////////////////////////////////

    void addBeginMarker();
    void addEndMarker();

    void startToggle();
    void endToggle();

    /** To be called for the undo/redo mechanism. */
    void change(long begin, long end, bool add);

    //////////////////////////////////////////////////////////////////////////
    // DRAWING
    //////////////////////////////////////////////////////////////////////////

    void draw(wxDC& dc);

private:

    GuiTimeLinePtr mTimeline;

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

    void onCursorMoved(EventTimelineCursorMoved& event);

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    wxRect makeRect(long x1, long x2);
    wxRect ptsToPixels(wxRect rect);
    void refresh(long begin, long end);

    //////////////////////////////////////////////////////////////////////////
    // SERIALIZATION 
    //////////////////////////////////////////////////////////////////////////

    friend class boost::serialization::access;
    template<class Archive> 
    void serialize(Archive & ar, const unsigned int version);
};

typedef boost::shared_ptr<SelectIntervals> IntervalsPtr;

}} // namespace

BOOST_CLASS_VERSION(gui::timeline::SelectIntervals, 1)
BOOST_CLASS_EXPORT(gui::timeline::SelectIntervals)

#endif // SELECT_INTERVALS_H
