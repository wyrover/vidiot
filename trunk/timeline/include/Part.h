#ifndef PART_H
#define PART_H

#include <wx/scrolwin.h>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

namespace model {
    class Sequence;
    typedef boost::shared_ptr<Sequence> SequencePtr;
}

namespace gui { 
    class Player;
    typedef boost::shared_ptr<Player> PlayerPtr;

    namespace timeline {
        class ClipView;
        class Cursor;
        class Divider;
        class Drag;
        class Player;
        class Intervals;
        class MenuHandler;
        class MousePointer;
        class Scrolling;
        class Selection;
        class VideoView;
        class AudioView;
        class Timeline;
        class SequenceView;
        class TrackView;
        class Tooltip;
        class ViewMap;
        class Zoom;

        namespace state { 
            class Machine; 
        } // namespace

class Part
    :   public boost::noncopyable
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION METHODS
    //////////////////////////////////////////////////////////////////////////

    Part(); ///< For constructing states
    Part(Timeline* timeline);
    virtual ~Part();

    //////////////////////////////////////////////////////////////////////////
    // PARTS
    //////////////////////////////////////////////////////////////////////////

    virtual Timeline& getTimeline(); ///< Virtual to be overridden in the statechart states
    virtual const Timeline& getTimeline() const; ///< Virtual to be overridden in the statechart states

    virtual wxScrolledWindow& getWindow(); ///< Virtual to be overridden in the statechart states
    virtual const wxScrolledWindow& getWindow() const; ///< Virtual to be overridden in the statechart states

    SequenceView& getSequenceView();
    const SequenceView& getSequenceView() const;

    Zoom& getZoom();
    const Zoom& getZoom() const;

    Intervals& getIntervals();
    const Intervals& getIntervals() const;

    Selection& getSelection();
    const Selection& getSelection() const;

    MousePointer& getMousePointer();
    const MousePointer& getMousePointer() const;

    Scrolling& getScrolling();
    const Scrolling& getScrolling() const;

    ViewMap& getViewMap();
    const ViewMap& getViewMap() const;

    MenuHandler& getMenuHandler();
    const MenuHandler& getMenuHandler() const;

    Cursor& getCursor();
    const Cursor& getCursor() const;

    Drag& getDrag();
    const Drag& getDrag() const;

    Tooltip& getTooltip();
    const Tooltip& getTooltip() const;

    Divider& getDivider();
    const Divider& getDivider() const;

    state::Machine& getStateMachine();
    const state::Machine& getStateMachine() const;

    //////////////////////////////////////////////////////////////////////////
    // OTHER HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    PlayerPtr getPlayer();

    model::SequencePtr getSequence();
    const model::SequencePtr getSequence() const;

private:

    Timeline * const mTimeline;
};

}} // namespace

#endif // PART_H
