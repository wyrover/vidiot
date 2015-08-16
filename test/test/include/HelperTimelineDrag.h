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

#ifndef HELPER_DRAG_H
#define HELPER_DRAG_H

namespace test {

struct DragParams
{
    DragParams();
    DragParams(const DragParams& other);

    /// Starting point of the drag.
    DragParams& From(wxPoint from); 

    /// Intermediate point of the drag.
    /// The drag is started by moving the drag to the 'from' point, then
    /// pressing the mouse button, and then moving to the 'via' point.
    /// If not explicitly specified, this point is calculated.
    DragParams& Via(wxPoint via);

    /// Ending point of the drag.
    DragParams& To(wxPoint to);

    /// Hold shift while dragging
    /// Shift is pressed after the drag has started starts
    /// Shift is released after the drag has been dropped
    DragParams& HoldShiftWhileDragging();

    /// The mouse press with which the drag is started, is surrounded with a controldown and controlup.
    DragParams& HoldCtrlBeforeDragStarts();

    DragParams& DontReleaseMouse();

    /// Do a drag to the left by the given amount of pixels
    DragParams& MoveLeft(pixel position);

    /// Do a drag to the left by the given amount of pixels
    DragParams& MoveRight(pixel position);

    /// Do a drag from the current position until the left  position of the dragged object is exactly on top of 'position'
    /// \param position point to which the left position of the drag must be aligned
    /// \note
    /// The current LeftPixel(DraggedClips()) may be slightly different than the requested position.
    /// When snapping is enabled, the snapping algorithm may cause a difference. If snapping is not
    /// enabled, the current zoom factor may cause differences. In fact, when snapping is not enabled
    /// not all pts values may be possible to 'drop on', since there's no matching pixel value. For
    /// instance when zoomed with a factor 5 (each pixel corresponds to 5 pts values), then only the
    /// pts values 1, and 5 are possible.
    DragParams& AlignLeft(pixel position);

    /// Do a drag from the current position until the right position of the dragged object is exactly on top of 'position'
    /// \param position point to which the right position of the drag must be aligned
    /// \see AlignLeft
    DragParams& AlignRight(pixel position);

    boost::optional<wxPoint> mFrom;
    boost::optional<wxPoint> mVia;
    boost::optional<wxPoint> mTo;
    bool mHoldShiftWhileDragging;
    bool mHoldCtrlBeforeDragStarts;
    bool mMouseUp;
    boost::optional<pixel> mAlignLeft;
    boost::optional<pixel> mAlignRight;

    friend std::ostream& operator<<(std::ostream& os, const DragParams& obj);
};

DragParams From(wxPoint from);          ///< \see DragParams::From
DragParams To(wxPoint to);              ///< \see DragParams::To
DragParams HoldShiftWhileDragging();    ///< \see DragParams::HoldShiftWhileDragging
DragParams HoldCtrlBeforeDragStarts();  ///< \see DragParams::HoldCtrlBeforeDragStarts
DragParams DontReleaseMouse();          ///< \see DragParams::DontReleaseMouse
DragParams AlignLeft(pixel position);   ///< \see DragParams::AlignLeft
DragParams AlignRight(pixel position);  ///< \see DragParams::AlignRight

/// Do a drag and drop between the two points (press, move, release).
/// \see DragParams class for usage
/// \note If the mouse is not yet down, it is pressed before the drag is started, AFTER moving to the 'from' point.
void TimelineDrag(const DragParams& params = DragParams());

/// Drag videoclip to VideoTrack(newtrackindex) and
/// Drag audioclip to AudioTrack(newtrackindex)
/// \param videoclip video clip to be moved to the video track with index newtrackindex
/// \param audioclip audio clip to be moved to the audio track with index newtrackindex
/// \param newtrackindex new track of clip
void TimelineDragToTrack(int newtrackindex, model::IClipPtr videoclip, model::IClipPtr audioclip);

} // namespace

#endif