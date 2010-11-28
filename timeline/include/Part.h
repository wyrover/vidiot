#ifndef TIMELINE_PART_H
#define TIMELINE_PART_H

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
        class Drag;
        class Drop;
        class GuiPlayer;
        class Intervals;
        class MenuHandler;
        class MousePointer;
        class Selection;
        class TimelineView;
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
    TimelineView& getView();

    //////////////////////////////////////////////////////////////////////////
    // OTHER HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    PlayerPtr getPlayer();
    model::SequencePtr getSequence();

    ////////////////////////////////////////////////////////////////////////////
    //// USED FOR VIEWS
    ////////////////////////////////////////////////////////////////////////////

    ///**
    //* Default implementation here ensures that not all derived classes have to
    //* do so.
    //**/
    //virtual void onViewUpdated( ViewUpdateEvent& event );

private:

    Timeline * const mTimeline;
};

}} // namespace

#endif // TIMELINE_PART_H
