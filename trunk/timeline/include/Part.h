#ifndef TIMELINE_PART_H
#define TIMELINE_PART_H

#include <boost/noncopyable.hpp>
#include "GuiPtr.h"
#include "ModelPtr.h"

namespace gui { namespace timeline {

class Part
    :   public boost::noncopyable
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION METHODS
    //////////////////////////////////////////////////////////////////////////

    Part();

    /** Initialized by the creator of the part (typically, the timeline) */
    void initTimeline(Timeline* timeline);

    /**
    * Called after the timeline pointer is initialized. Parts do their 
    * initializing here, specifically at this point the get*() methods 
    * of this superclass are available which is not the case in the
    * constructor. */
    virtual void init();

    virtual ~Part();

    //////////////////////////////////////////////////////////////////////////
    // PARTS
    //////////////////////////////////////////////////////////////////////////

    /** Virtual to be overridden in the statechart states. */
    virtual Timeline& getTimeline();
    const Timeline& getTimeline() const;
    Zoom& getZoom();
    const Zoom& getZoom() const;
    Intervals& getIntervals();
    Selection& getSelection();
    MousePointer& getMousePointer();
    ViewMap& getViewMap();
    MenuHandler& getMenuHandler();
    Cursor& getCursor();
    Drag& getDrag();
    Drop& getDrop();
    SequenceView& getSequenceView();

    //////////////////////////////////////////////////////////////////////////
    // OTHER HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    PlayerPtr getPlayer();
    model::SequencePtr getSequence();

private:

    Timeline* mTimeline;
};

}} // namespace

#endif // TIMELINE_PART_H
