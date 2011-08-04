#ifndef HELPER_TIMELINE_H
#define HELPER_TIMELINE_H

#include <wx/gdicmn.h>
#include <boost/shared_ptr.hpp>
#include "UtilInt.h"

namespace model {
class IClip;
typedef boost::shared_ptr<IClip> IClipPtr;
class VideoTrack;
typedef boost::shared_ptr<VideoTrack> VideoTrackPtr;
class AudioTrack;
typedef boost::shared_ptr<AudioTrack> AudioTrackPtr;
}

namespace test {

wxPoint TimelinePosition();                 ///< \return the physical position of the active timeline on the screen

/// Return the number of clips in a given video track
/// \param trackindex index position (0-based) of the video track, counting from the divider upwards
int NumberOfVideoClipsInTrack(int trackindex = 0);

/// Return a video track
/// \param trackindex index position (0-based) of the video track, counting from the divider upwards
model::VideoTrackPtr VideoTrack(int trackindex = 0);

/// Return a audio track
/// \param trackindex index position (0-based) of the audio track, counting from the divider downwards
model::AudioTrackPtr AudioTrack(int trackindex = 0);

/// Return a clip in a video track
/// \param trackindex index position (0-based) of the video track, counting from the divider upwards
/// \param clipindex index position (0-based) of the clip in the track, counting from left to right
model::IClipPtr VideoClip(int trackindex = 0, int clipindex = 0);

/// Return a clip in an audio track
/// \param trackindex index position (0-based) of the audio track, counting from the divider upwards
/// \param clipindex index position (0-based) of the clip in the track, counting from left to right
model::IClipPtr AudioClip(int trackindex = 0, int clipindex = 0);

int getNonEmptyClipsCount();                ///< \return number of non-empty clips (both audio and video) in given timeline
int getSelectedClipsCount();                ///< \return number of selected clips (both audio and video) in given timeline

pixel LeftPixel(model::IClipPtr clip);      ///< \return left x position of given clip
pixel RightPixel(model::IClipPtr clip);     ///< \return right x position of given clip
pixel TopPixel(model::IClipPtr clip);       ///< \return top y position of given clip
pixel BottomPixel(model::IClipPtr clip);    ///< \return bottom y position of given clip

wxPoint Center(model::IClipPtr clip);       ///< \return center (pixel) position of a clip
wxPoint LeftCenter(model::IClipPtr clip);   ///< \return left center position (centered vertically)
wxPoint RightCenter(model::IClipPtr clip);  ///< \return right center position (centered vertically)

void PositionCursor(pixel position);                                    ///< Move the mouse and then click the left button, in order to move the cursor line to the given position
void Click(model::IClipPtr clip);                                       ///< Click (down+up) the mouse in the center of a clip
void TrimLeft(model::IClipPtr clip, pixel length, bool shift = true);   ///< Trim the given clip on the left side
void TrimRight(model::IClipPtr clip, pixel length, bool shift = true);  ///< Trim the given clip on the right side
void Drag(wxPoint from, wxPoint to, bool ctrl = false);                 ///< Do a drag and drop between the two points (press, move, release). If ctrl = true, then ctrl is pressed at the beginning of the drag (is released directly after moving 'a bit')

/// Assert for the count of the selected clips.
/// Named such for readibility of test cases.
/// Note that the current selected clips (getSelectedClipsCount())
/// are compared to size * 2 (to avoid having to duplicate for
/// the combination of audio and video clips throughout the tests).
void ASSERT_SELECTION_SIZE(int size);

/// Unselect all clips in the timeline
void DeselectAllClips();

/// Dump the sequence's contents
void DumpSequence();

} // namespace

#endif // HELPER_TIMELINE_H