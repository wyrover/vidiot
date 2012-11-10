#ifndef HELPER_DRAG_H
#define HELPER_DRAG_H

#include <boost/shared_ptr.hpp>
#include <wx/gdicmn.h>
#include "UtilInt.h"
#include <ostream>

namespace model {
class IClip;
typedef boost::shared_ptr<IClip> IClipPtr;
}

namespace test {

struct DragParams
{
    DragParams();
    DragParams(const DragParams& other);

    DragParams& From(wxPoint from);
    DragParams& To(wxPoint to);

    /// Hold shift while dragging
    /// Shift is pressed after the drag has started starts
    /// Shift is released after the drag has been dropped
    DragParams& HoldShiftWhileDragging();

    /// The mouse press with which the drag is started, is surrounded with a controldown and controlup.
    DragParams& HoldCtrlBeforeDragStarts();

    DragParams& DontReleaseMouse();

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
    boost::optional<wxPoint> mTo;
    bool mHoldShiftWhileDragging;
    bool mHoldCtrlBeforeDragStarts;
    bool mMouseUp;
    boost::optional<pixel> mAlignLeft;
    boost::optional<pixel> mAlignRight;

    friend std::ostream& operator<<( std::ostream& os, const DragParams& obj );
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
void Drag(const DragParams& params = DragParams());

/// Drag videoclip to VideoTrack(newtrackindex) and
/// Drag audioclip to AudioTrack(newtrackindex)
/// \param videoclip video clip to be moved to the video track with index newtrackindex
/// \param audioclip audio clip to be moved to the audio track with index newtrackindex
/// \param newtrackindex new track of clip
void DragToTrack(int newtrackindex, model::IClipPtr videoclip, model::IClipPtr audioclip);

} // namespace

#endif // HELPER_DRAG_H