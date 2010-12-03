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
    class GuiPlayer;
    typedef boost::shared_ptr<GuiPlayer> PlayerPtr;

    namespace timeline {
        class ClipView;
        class Cursor;
        class Divider;
        class Drag;
        class Drop;
        class GuiPlayer;
        class Intervals;
        class MenuHandler;
        class MousePointer;
        class Selection;
        class VideoView;
        class AudioView;
        class Timeline;
        class TrackView;
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

    Zoom& getZoom();
    const Zoom& getZoom() const;

    Intervals& getIntervals();
    const Intervals& getIntervals() const;

    Selection& getSelection();
    const Selection& getSelection() const;

    MousePointer& getMousePointer();
    const MousePointer& getMousePointer() const;

    ViewMap& getViewMap();
    const ViewMap& getViewMap() const;

    MenuHandler& getMenuHandler();
    const MenuHandler& getMenuHandler() const;

    Cursor& getCursor();
    const Cursor& getCursor() const;

    Drag& getDrag();
    const Drag& getDrag() const;

    Drop& getDrop();
    const Drop& getDrop() const;

    Divider& getDivider();
    const Divider& getDivider() const;

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
