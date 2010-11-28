#ifndef TIMELINE_PART_H
#define TIMELINE_PART_H

#include <boost/noncopyable.hpp>
#include "GuiPtr.h"
#include "ModelPtr.h"

namespace gui { namespace timeline {

class ViewUpdateEvent;

class Part
    :   public boost::noncopyable
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION METHODS
    //////////////////////////////////////////////////////////////////////////

    /** For states */
    Part();

    Part(Timeline* timeline);

    virtual ~Part();

    //////////////////////////////////////////////////////////////////////////
    // PARTS
    //////////////////////////////////////////////////////////////////////////

    /** Virtual to be overridden in the statechart states and in Timeline. */
    virtual Timeline& getTimeline();
    virtual const Timeline& getTimeline() const;
    virtual Zoom& getZoom();
    virtual const Zoom& getZoom() const;
    virtual Intervals& getIntervals();
    virtual Selection& getSelection();
    virtual MousePointer& getMousePointer();
    virtual ViewMap& getViewMap();
    virtual MenuHandler& getMenuHandler();
    virtual Cursor& getCursor();
    virtual Drag& getDrag();
    virtual Drop& getDrop();
    virtual TimelineView& getView();

    //////////////////////////////////////////////////////////////////////////
    // OTHER HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    PlayerPtr getPlayer();
    model::SequencePtr getSequence();

    //////////////////////////////////////////////////////////////////////////
    // USED FOR VIEWS
    //////////////////////////////////////////////////////////////////////////

    /**
    * Default implementation here ensures that not all derived classes have to
    * do so.
    **/
    virtual void onViewUpdated( ViewUpdateEvent& event );

private:

    Timeline * const mTimeline;
};

}} // namespace

#endif // TIMELINE_PART_H
