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
    /// \position mouse position on screen. Do not replace with virtual position since the virtual canvas is changed because of shift trimming and keeping one clip edge aligned.
    void update(wxPoint position);

    /// Abort a pending trim operation. If changes were made, undo them.
    void stop();

    /// Submit any pending trim operation. Do not undo the change that was done.
    /// Instead, ensure that the operation becomes undoable.
    void submit();

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    bool mActive;                   ///< True if a trim is currently being done
    wxPoint mStartPosition;         ///< Mouse position (in unscrolled coordinates) when the trimming was started

    command::TrimClip* mCommand;    ///< The command that executes the Trim operation

    pts mFixedPts;                  ///< Pts value (in the track) that must be kept at a fixed pixel position. Used for keeping the left/right position of the clip fixed as much as possible when shift trimming.
    pixel mFixedPixel;              ///< Pixel value (physical) that mFixedPts must be aligned with. Used for keeping the left/right position of the clip fixed as much as possible when shift trimming.

    boost::shared_ptr<wxBitmap> mAdjacentBitmap;
    model::VideoClipPtr mPreviewVideoClip;
    wxMemoryDC mDc;
    pts mStartPositionPreview;
    wxBitmapPtr mBitmapSingle;
    wxBitmapPtr mBitmapSideBySide;

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    void preview();
    void previewThread();
};

}} // namespace

#endif // TRIM_H