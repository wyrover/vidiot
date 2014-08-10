// Copyright 2013,2014 Eric Raijmakers.
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

#include "Test.h"

namespace test {

    DragParams::DragParams()
        :   mFrom(boost::none)
        ,   mTo(boost::none)
        ,   mHoldShiftWhileDragging(false)
        ,   mHoldCtrlBeforeDragStarts(false)
        ,   mMouseUp(true)
    {}

    DragParams::DragParams(const DragParams& other)
        :   mFrom(other.mFrom)
        ,   mTo(other.mTo)
        ,   mHoldShiftWhileDragging(other.mHoldShiftWhileDragging)
        ,   mHoldCtrlBeforeDragStarts(other.mHoldCtrlBeforeDragStarts)
        ,   mMouseUp(other.mMouseUp)
    {}

    DragParams& DragParams::From(wxPoint from)         { mFrom.reset(from);                return *this; }
    DragParams& DragParams::To(wxPoint to)             { mTo.reset(to);                    return *this; }
    DragParams& DragParams::HoldShiftWhileDragging()   { mHoldShiftWhileDragging = true;   return *this; }
    DragParams& DragParams::HoldCtrlBeforeDragStarts() { mHoldCtrlBeforeDragStarts = true; return *this; }
    DragParams& DragParams::DontReleaseMouse()         { mMouseUp = false;                 return *this; }
    DragParams& DragParams::AlignLeft(pixel position)  { mAlignLeft.reset(position);       return *this; }
    DragParams& DragParams::AlignRight(pixel position) { mAlignRight.reset(position);      return *this; }
    DragParams& DragParams::MoveLeft(pixel position)
    {
        ASSERT(mFrom && !mTo);
        mTo.reset(*mFrom + wxPoint(-position,0));
        return *this;
    }
    DragParams& DragParams::MoveRight(pixel position)
    {
        ASSERT(mFrom && !mTo);
        mTo.reset(*mFrom + wxPoint(position,0));
        return *this;
    };

    boost::optional<wxPoint> mFrom;
    boost::optional<wxPoint> mTo;
    bool mHoldShiftWhileDragging;
    bool mHoldCtrlBeforeDragStarts;
    bool mMouseUp;
    boost::optional<pixel> mAlignLeft;
    boost::optional<pixel> mAlignRight;

    std::ostream& operator<<(std::ostream& os, const DragParams& obj)
    {
#define VERBOSE(param) #param << '=' << param
        //os << VERBOSE(obj.mFrom) << VERBOSE(obj.mTo) << std::boolalpha << VERBOSE(obj.mHoldShiftWhileDragging) << VERBOSE(obj.mHoldCtrlBeforeDragStarts) << VERBOSE(obj.mMouseUp);
        return os;
    };

DragParams From(wxPoint from)         { return DragParams().From(from); }
DragParams To(wxPoint to)             { return DragParams().To(to); }
DragParams HoldShiftWhileDragging()   { return DragParams().HoldShiftWhileDragging(); }
DragParams HoldCtrlBeforeDragStarts() { return DragParams().HoldCtrlBeforeDragStarts(); }
DragParams DontReleaseMouse()         { return DragParams().DontReleaseMouse(); }
DragParams AlignLeft(pixel position)  { return DragParams().AlignLeft(position); }
DragParams AlignRight(pixel position) { return DragParams().AlignRight(position); }

void TimelineDrag(const DragParams& params)
{
    VAR_DEBUG(params);
    ASSERT(!params.mHoldShiftWhileDragging || !params.mHoldCtrlBeforeDragStarts); // Can't handle both in one action (at least, never tested this)
    ASSERT(!params.mAlignLeft || !params.mAlignRight); // Can't align both sides
    ASSERT_IMPLIES(params.mHoldCtrlBeforeDragStarts,!getTimeline().getMouse().getLeftDown()); // Holding CTRL can only be done for the first (mouse down) action in a sequence of drag related actions
    ASSERT_IMPLIES(params.mHoldCtrlBeforeDragStarts,!getTimeline().getKeyboard().getCtrlDown());
    ASSERT_IMPLIES(params.mHoldCtrlBeforeDragStarts, !getTimeline().getDrag().isActive());
    ASSERT(params.mFrom);
    ASSERT_IMPLIES(params.mTo, !params.mAlignLeft && !params.mAlignRight);
    ASSERT(params.mTo || params.mAlignLeft || params.mAlignRight);

    // Press down mouse button. Is done when the mouse is not yet pressed.
    wxPoint from = *params.mFrom;
    if (params.mHoldCtrlBeforeDragStarts) { TimelineKeyDown(WXK_CONTROL); }
    TimelineMove(from);
    if (!getTimeline().getMouse().getLeftDown()) { TimelineLeftDown(); }
    if (params.mHoldCtrlBeforeDragStarts) { TimelineKeyUp(WXK_CONTROL); }

    wxPoint between(from);
    if (!getTimeline().getDrag().isActive())
    {
        // Drag a bit until the drag is started
        if (params.mTo)
        {
            wxPoint to(*params.mTo);
            if ( from.x != to.x )
            {
                // Only in case the actually requested drag and drop operation includes a motion in x-direction, the x-direction
                // may be used here. Otherwise, this initial drag will cause 'snapping' to zoomed pts values (for instance, if the zoom
                // is such that 1 pixel equals 5 pts positions). In case of y-only drag and drops it is important to never move the
                // mouse in x-direction.
                between.x += (from.x > to.x) ? -(gui::Layout::DragThreshold+1) : (gui::Layout::DragThreshold+1); // Should be greater than the tolerance in StateLeftDown (otherwise, the Drag won't be started)
            }
            else
            {
                between.y += (from.y > to.y) ? -(gui::Layout::DragThreshold+1) : (gui::Layout::DragThreshold+1); // Should be greater than the tolerance in StateLeftDown (otherwise, the Drag won't be started)
            }
        }
        else
        {
            //  (params.mAlignLeft || params.mAlignRight)
            between.x += gui::Layout::DragThreshold + 1; // Should be greater than the tolerance in StateLeftDown (otherwise, the Drag won't be started)
        }
        TimelineMove(between);
    }
    ASSERT(getTimeline().getDrag().isActive());

    // Determine the target point
    wxPoint to(0,0);
    if (params.mAlignLeft || params.mAlignRight)
    {
        bool left = params.mAlignLeft;
        pixel alignposition = params.mAlignLeft ? LeftPixel(DraggedClips()) : RightPixel(DraggedClips()); // Requires active drag
        pixel position = params.mAlignLeft ? *params.mAlignLeft : *params.mAlignRight;
        to = between;
        to.x += (position - alignposition);
    }
    else
    {
        to = *params.mTo;
    }

    // Press shift while moving
    if (params.mHoldShiftWhileDragging) { TimelineKeyDown(WXK_SHIFT); }

    // Drop onto target point
    TimelineMove(to);
    if (params.mMouseUp)
    {
        TimelineLeftUp();
        ASSERT(!getTimeline().getDrag().isActive());
    }
    if (params.mHoldShiftWhileDragging) { TimelineKeyUp(WXK_SHIFT); }
}

void TimelineDragToTrack(int newtrackindex, model::IClipPtr videoclip, model::IClipPtr audioclip)
{
    if (videoclip)
    {
        DragParams params = From(Center(videoclip)).To(wxPoint(HCenter(videoclip),VCenter(VideoTrack(newtrackindex))));
        if (audioclip)
        {
            params.DontReleaseMouse();
        }
        TimelineDrag(params);
    }
    if (videoclip && audioclip)
    {
        TimelineKeyDown(WXK_CONTROL);
        TimelineMove(Center(audioclip));
        TimelineKeyUp(WXK_CONTROL);
    }
    if (audioclip)
    {
        TimelineDrag(From(Center(audioclip)).To(wxPoint(HCenter(audioclip),VCenter(AudioTrack(newtrackindex)))));
    }
}

} // namespace
