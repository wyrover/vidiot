// Copyright 2013-2015 Eric Raijmakers.
//
// This file is part of Vidiot.
//
// Vidiot is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Vidiot is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Vidiot. If not, see <http://www.gnu.org/licenses/>.

#ifndef PART_H
#define PART_H

namespace gui {
    class Player;

    namespace timeline {
        class AudioView;
        class ClipView;
        class Cursor;
        class Details;
        class Divider;
        class Drag;
        class Intervals;
        class Keyboard;
        class MenuHandler;
        class Mouse;
        class Scrolling;
        class Selection;
        class SequenceView;
        class ThumbnailView;
        class Timeline;
        class TimelineClipboard;
        class Tooltip;
        class TrackView;
        class Trim;
        class VideoView;
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

    SequenceView& getSequenceView();
    const SequenceView& getSequenceView() const;

    Zoom& getZoom();
    const Zoom& getZoom() const;

    Intervals& getIntervals();
    const Intervals& getIntervals() const;

    Keyboard& getKeyboard();
    const Keyboard& getKeyboard() const;

    TimelineClipboard& getClipboard();
    const TimelineClipboard& getClipboard() const;

    Selection& getSelection();
    const Selection& getSelection() const;

    Mouse& getMouse();
    const Mouse& getMouse() const;

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

    Trim& getTrim();
    const Trim& getTrim() const;

    state::Machine& getStateMachine();
    const state::Machine& getStateMachine() const;

    Details& getDetails();
    const Details& getDetails() const;

    //////////////////////////////////////////////////////////////////////////
    // OTHER HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    Player* getPlayer();

    model::SequencePtr getSequence();
    const model::SequencePtr getSequence() const;

private:

    Timeline * const mTimeline;
};

}} // namespace

#endif
