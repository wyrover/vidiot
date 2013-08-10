// Copyright 2013 Eric Raijmakers.
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

#ifndef TRIM_H
#define TRIM_H

#include "Part.h"
#include "PositionInfo.h"
#include "UtilInt.h"

namespace gui {
    class EditDisplay;
}

namespace gui { namespace timeline {

struct EvLeftUp;
struct EvLeave;
struct EvMotion;
struct EvKeyUp;
struct EvKeyDown;

namespace command {
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
    ~Trim();

    //////////////////////////////////////////////////////////////////////////
    // TRIM OPERATION
    //////////////////////////////////////////////////////////////////////////

    /// Start trimming
    void start();

    /// Do an update of the trim operation. Must be called upon relevant
    /// mouse/keyboard changes.
    void update();

    /// Abort a pending trim operation. If changes were made, undo them.
    void stop();

    /// Submit any pending trim operation. Do not undo the change that was done.
    /// Instead, ensure that the operation becomes undoable.
    void submit();

    void draw(wxDC& dc) const;

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    bool mActive;                   ///< True if a trim is currently being done
    wxPoint mStartPosition;         ///< Mouse position (in unscrolled coordinates) when the trimming was started
    MouseOnClipPosition mPosition;  ///< Logical positin where the trimming was started
    pts mStartPts;                  ///< Position (in pts values) when the trimming was started

    command::TrimClip* mCommand;    ///< The command that executes the Trim operation

    pts mFixedPts;                  ///< Pts value (in the track) that must be kept at a fixed pixel position. Used for keeping the left/right position of the clip fixed as much as possible when shift trimming.
    pixel mFixedPixel;              ///< Pixel value (physical) that mFixedPts must be aligned with. Used for keeping the left/right position of the clip fixed as much as possible when shift trimming.

    std::list<pts> mSnapPoints;     ///< Sorted list containing all possible 'snap to' points (pts values). Filled upon start of trim.
    boost::optional<pts> mSnap;     ///< Current snapping position (that is, where the trim position 'touches' the pts position of another clip)

    boost::shared_ptr<wxBitmap> mAdjacentBitmap;
    model::VideoClipPtr mPreviewVideoClip;
    wxMemoryDC mDc;
    pts mStartPositionPreview;
    wxBitmapPtr mBitmapSingle;
    wxBitmapPtr mBitmapSideBySide;

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    pts determineTrimDiff(); ///< \return the trim diff to be applied, including (optional) snapping to clips or to the cursor.
    void determinePossibleSnapPoints(model::IClipPtr originalclip);
    void preview();
};

}} // namespace

#endif // TRIM_H