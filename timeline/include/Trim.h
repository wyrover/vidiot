// Copyright 2013-2016 Eric Raijmakers.
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

#pragma once

#include "Part.h"
#include "PositionInfo.h"

namespace gui {
    class EditDisplay;
}

namespace gui { namespace timeline {

struct EvLeftUp;
struct EvLeave;
struct EvMotion;
struct EvKeyUp;
struct EvKeyDown;

namespace cmd {
    class TrimClip;
}

/// Class responsible for any previewing during the edit operation. This includes
/// changing the timeline contents (via the command) and the preview in the player
/// window. Finally, this class executes the operation via the submit of the command.
class Trim
    :   public wxEvtHandler // MUST BE FIRST INHERITED CLASS FOR WXWIDGETS EVENTS TO BE RECEIVED.
    ,   public Part
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    Trim(Timeline* timeline);
    virtual ~Trim();

    //////////////////////////////////////////////////////////////////////////
    // TRIM OPERATION
    //////////////////////////////////////////////////////////////////////////

    /// Start trimming
    void start();

    /// Do an update of the trim operation. Must be called upon relevant
    /// mouse/keyboard changes.
    void update();

    void toggleSnapping(); ///< Temporarily disable/enable snap-to-whatever

    void toggleTrimLink(); ///< Temporarily disable/enable trimming the linked clip also.

    /// Abort a pending trim operation. If changes were made, undo them.
    void stop();

    /// Submit any pending trim operation. Do not undo the change that was done.
    /// Instead, ensure that the operation becomes undoable.
    void submit();

    void drawSnaps(wxDC& dc, const wxRegion& region, const wxPoint& offset) const;

    bool isActive() const;

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    bool mActive = false;                       ///< True if a trim is currently being done
    wxPoint mStartPosition;                     ///< Mouse position (in unscrolled coordinates) when the trimming was started
    MouseOnClipPosition mPosition;              ///< Logical positin where the trimming was started
    pts mStartPts = 0;                          ///< Position (in pts values) when the trimming was started
    bool mSnappingEnabled = true;               ///< Used to overrule snapping during a trim operation.
    bool mTrimLink = false;                     ///< Used to overrule trimming the link simultaneously.

    cmd::TrimClip* mCommand = nullptr;          ///< The command that executes the Trim operation

    pts mFixedPts = 0;                          ///< Pts value (in the track) that must be kept at a fixed pixel position. Used for keeping the left/right position of the clip fixed as much as possible when shift trimming.
    pixel mFixedPixel = 0;                      ///< Pixel value (physical) that mFixedPts must be aligned with. Used for keeping the left/right position of the clip fixed as much as possible when shift trimming.

    std::vector<pts> mSnapPoints;               ///< Sorted list containing all possible 'snap to' points (pts values). Filled upon start of trim.
    boost::optional<pts> mSnap = boost::none;   ///< Current snapping position (that is, where the trim position 'touches' the pts position of another clip)

    pts mCursorPositionBefore = 0;              ///< Cursor position before the trim is started.

    model::VideoClipPtr mVideoClip = nullptr;   ///< Videoclip to be used for showing a preview of the trim operation.
    wxBitmapPtr mAdjacentBitmap = nullptr;      ///< Adjacent bitmap to be used for showing a preview of a shift-trim operation.
    wxMemoryDC mDc;
    pts mStartPositionPreview = 0;
    wxBitmapPtr mBitmapSingle = nullptr;
    wxBitmapPtr mBitmapSideBySide = nullptr;

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    pts determineTrimDiff(); ///< \return the trim diff to be applied, including (optional) snapping to clips or to the cursor.
    void determinePossibleSnapPoints(const model::IClipPtr& originalclip);
    void preview();
};

}} // namespace
