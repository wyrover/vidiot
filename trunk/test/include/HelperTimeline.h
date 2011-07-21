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

    
    wxPoint TimelinePosition();                 ///< \return the physical position of the active timeline on the screen

    /// Return the number of clips in a given video track
    /// \param trackindex index position (0-based) of the video track, counting from the divider upwards
    int NumberOfVideoClipsInTrack(int trackindex = 0);

    /// Return a clip in a video track
    /// \param trackindex index position (0-based) of the video track, counting from the divider upwards
    /// \param clipindex index position (0-based) of the clip in the track, counting from left to right
    model::IClipPtr VideoClip(int trackindex = 0, int clipindex = 0);
    
    int getNonEmptyClipsCount();                ///< \return number of non-empty clips (both audio and video) in given timeline
    int getSelectedClipsCount();                ///< \return number of selected clips (both audio and video) in given timeline
    
    pixel LeftPixel(model::IClipPtr clip);      ///< \return left x position of given clip
    pixel RightPixel(model::IClipPtr clip);     ///< \return right x position of given clip
    pixel TopPixel(model::IClipPtr clip);       ///< \return top y position of given clip
    pixel BottomPixel(model::IClipPtr clip);    ///< \return bottom y position of given clip
    
    wxPoint Center(model::IClipPtr clip);       ///< \return center (pixel) position of a clip
    wxPoint LeftCenter(model::IClipPtr clip);   ///< \return left center position (centered vertically)
    wxPoint RightCenter(model::IClipPtr clip);  ///< \return right center position (centered vertically)
    
    void Click(model::IClipPtr clip);                                       ///< Click (down+up) the mouse in the center of a clip
    void TrimLeft(model::IClipPtr clip, pixel length, bool shift = true);   ///< Trim the given clip on the left side
    void TrimRight(model::IClipPtr clip, pixel length, bool shift = true);  ///< Trim the given clip on the right side
    
    /// Assert for the count of the selected clips.
    /// Named such for readibility of test cases.
    void ASSERT_SELECTION_SIZE(int size);

protected:

    wxFileName TestFilesPath;               ///< Contains the path to the test files
    model::IPaths InputFiles;               ///< Contains the filenames of the input files in the test directory

};

} // namespace

#endif // HELPER_TIMELINE_H