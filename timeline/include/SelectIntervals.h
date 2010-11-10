#ifndef SELECT_INTERVALS_H
#define SELECT_INTERVALS_H

#include <map>
#include <wx/dc.h>
#include <wx/event.h>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/version.hpp>
#include "ModelPtr.h"
#include "GuiPtr.h"
#include "TimeLinePart.h"

namespace gui { namespace timeline {

class EventTimelineCursorMoved;

class SelectIntervals
    :   public wxEvtHandler
    ,   public TimeLinePart
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION METHODS
    //////////////////////////////////////////////////////////////////////////

    SelectIntervals();
    virtual void init();
    virtual ~SelectIntervals();

    //////////////////////////////////////////////////////////////////////////
    // MARKING / TOGGLING INTERFACE
    //////////////////////////////////////////////////////////////////////////

    wxRegion get();
    void set(wxRegion region);

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

    void OnDeleteMarked(wxCommandEvent& WXUNUSED(event));
    void OnDeleteUnmarked(wxCommandEvent& WXUNUSED(event));
    void OnRemoveMarkers(wxCommandEvent& WXUNUSED(event));

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    wxRect makeRect(long x1, long x2);
    wxRect ptsToPixels(wxRect rect);
    void refresh(long begin, long end);
    void updateMenu();

    typedef std::map< model::ClipPtr, model::ClipPtr > ReplacementMap;
    ReplacementMap findReplacements(GuiTimeLineTrack* track);

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
BOOST_CLASS_VERSION(gui::timeline::SelectIntervals, 1)
BOOST_CLASS_EXPORT(gui::timeline::SelectIntervals)

#endif // SELECT_INTERVALS_H
