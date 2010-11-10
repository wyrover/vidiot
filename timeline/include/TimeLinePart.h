#ifndef TIMELINE_PART_H
#define TIMELINE_PART_H

#include <boost/noncopyable.hpp>
#include "GuiPtr.h"
#include "ModelPtr.h"

namespace gui { namespace timeline {

class GuiTimeLine;
class GuiTimeLineZoom;
class SelectIntervals;
class SelectClips;
class GuiPlayer;
class MousePointer;
class ViewMap;

class TimeLinePart
    :   public boost::noncopyable
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION METHODS
    //////////////////////////////////////////////////////////////////////////

    TimeLinePart();

    /** Initialized by the creator of the part (typically, the timeline) */
    void initTimeline(GuiTimeLine* timeline);

    /**
    * Called after the timeline pointer is initialized. Parts do their 
    * initializing here, specifically at this point the get*() methods 
    * of this superclass are available which is not the case in the
    * constructor. */
    virtual void init();

    virtual ~TimeLinePart();

    //////////////////////////////////////////////////////////////////////////
    // PARTS
    //////////////////////////////////////////////////////////////////////////

    /** Virtual to be overridden in the statechart states. */
    virtual GuiTimeLine& getTimeline();
    GuiTimeLineZoom& getZoom();
    SelectIntervals& getSelectIntervals();
    SelectClips& getSelectClips();
    MousePointer& getMousePointer();
    ViewMap& getViewMap();

    //////////////////////////////////////////////////////////////////////////
    // OTHER HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    PlayerPtr getPlayer();
    model::SequencePtr getSequence();

private:

    GuiTimeLine* mTimeline;
};

}} // namespace

#endif // TIMELINE_PART_H
