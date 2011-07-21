#ifndef HELPER_TIMELINE_H
#define HELPER_TIMELINE_H

#include <list>
#include <wx/filename.h>
#include <wx/uiaction.h>
#include <boost/shared_ptr.hpp>
#include "UtilInt.h"

namespace model {
class IClip;
typedef boost::shared_ptr<IClip> IClipPtr;
class IPath;
typedef boost::shared_ptr<IPath> IPathPtr;
typedef std::list<IPathPtr> IPaths;

}

namespace test {

/// Base class for timeline tests. Contains helper methods for dealing with the active timeline.
class HelperTimeline
    :    public wxUIActionSimulator
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    HelperTimeline();
    ~HelperTimeline();

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    /// Return the physical position of the active timeline on the screen
    wxPoint TimelinePosition();

    /// Return the number of clips in a given video track
    /// \param trackindex index position (0-based) of the video track, counting from the divider upwards
    int NumberOfVideoClipsInTrack(int trackindex = 0);

    /// Return a clip in a video track
    /// \param trackindex index position (0-based) of the video track, counting from the divider upwards
    /// \param clipindex index position (0-based) of the clip in the track, counting from left to right
    model::IClipPtr VideoClip(int trackindex = 0, int clipindex = 0);
    
    /// Count non-empty clips (both audio and video) in given timeline
    int getNonEmptyClipsCount();

    /// Count selected clips (both audio and video) in given timeline
    int getSelectedClipsCount();

    /// Return left x position of given clip
    /// \param clip given clip
    pixel LeftPixel(model::IClipPtr clip);

    /// Return right x position of given clip
    /// \param clip given clip
    pixel RightPixel(model::IClipPtr clip);

    /// Return top y position of given clip
    /// \param clip given clip
    pixel TopPixel(model::IClipPtr clip);

    /// Return bottom y position of given clip
    /// \param clip given clip
    pixel BottomPixel(model::IClipPtr clip);

    /// Clicks the mouse in the center of a clip
    /// \clip clip to be clicked
    void click(model::IClipPtr clip);

    /// Assert for the count of the selected clips.
    /// Named such for readibility of test cases.
    void ASSERT_SELECTION_SIZE(int size);

protected:

    /// Contains the path to the test files
    wxFileName TestFilesPath;

    /// Contains the filenames of the input files in the test directory
    model::IPaths InputFiles;

};

} // namespace

#endif // HELPER_TIMELINE_H