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
class VideoTransition;
typedef boost::shared_ptr<VideoTransition> VideoTransitionPtr;
class AudioTrack;
typedef boost::shared_ptr<AudioTrack> AudioTrackPtr;
}

namespace gui { namespace timeline {
    enum MouseOnClipPosition;
}}

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

/// Return a video transition in a video track
/// \param trackindex index position (0-based) of the video track, counting from the divider upwards
/// \param clipindex index position (0-based) of the clip in the track, counting from left to right
/// \pre the clip at the given position is a transitin
model::VideoTransitionPtr VideoTransition(int trackindex, int clipindex);

/// Return a clip in an audio track
/// \param trackindex index position (0-based) of the audio track, counting from the divider upwards
/// \param clipindex index position (0-based) of the clip in the track, counting from left to right
model::IClipPtr AudioClip(int trackindex = 0, int clipindex = 0);

int getNonEmptyClipsCount();                ///< \return number of non-empty clips (both audio and video) in given timeline
int getSelectedClipsCount();                ///< \return number of selected clips (both audio and video) in given timeline

pixel LeftPixel(model::IClipPtr clip);          ///< \return left x position of given clip
pixel RightPixel(model::IClipPtr clip);         ///< \return right x position of given clip
pixel TopPixel(model::IClipPtr clip);           ///< \return top y position of given clip
pixel BottomPixel(model::IClipPtr clip);        ///< \return bottom y position of given clip
pixel VCenter(model::IClipPtr clip);            ///< \return vertical center position of given clip
pixel VQuarter(model::IClipPtr clip);           ///< \return y position at one quarter from the top
pixel HCenter(model::IClipPtr clip);            ///< \return horizontal center position of given clip

wxPoint Center(model::IClipPtr clip);           ///< \return center (pixel) position of a clip
wxPoint VQuarterHCenter(model::IClipPtr clip);  ///< \return pixel at one quarter from the top (centered horizontally)
wxPoint LeftCenter(model::IClipPtr clip);       ///< \return left center position (centered vertically)
wxPoint RightCenter(model::IClipPtr clip);      ///< \return right center position (centered vertically)

void PositionCursor(pixel position);                                    ///< Move the mouse and then click the left button, in order to move the cursor line to the given position
void Move(wxPoint position);                                            ///< Move the mouse to the given position within the timeline
void Click(wxPoint position);                                           ///< Move the mouse to the given position and (left) click there
void TrimLeft(model::IClipPtr clip, pixel length, bool shift = true);   ///< Trim the given clip on the left side
void TrimRight(model::IClipPtr clip, pixel length, bool shift = true);  ///< Trim the given clip on the right side

/// Do a drag and drop between the two points (press, move, release).
/// \param from starting position to move to initially
/// \param to final position to drag to
/// \param ctrl if true, then ctrl is pressed at the beginning of the drag and released directly after moving 'a bit'
/// \param mousedown if true, then the mouse button is pressed just after moving to 'from' and before initiating the drag
/// \param mouseup if true, then the mouse button is released after the mouse has reached position 'to'
void Drag(wxPoint from, wxPoint to, bool ctrl = false, bool mousedown = true, bool mouseup = true);

void Scrub(pixel from, pixel to);                                       ///< Scrub the cursor over the timeline (view frames in preview window)
void Play(pixel from, int ms);                                          ///< Play from the given position, with the given time.

gui::timeline::MouseOnClipPosition LogicalPosition(wxPoint position);   ///< \return logical mouse position of the given (pixel) position.

/// Assert for the count of the selected clips.
/// Named such for readibility of test cases.
/// Note that the current selected clips (getSelectedClipsCount())
/// are compared to size * 2 (to avoid having to duplicate for
/// the combination of audio and video clips throughout the tests).
void ASSERT_SELECTION_SIZE(int size);

/// Unselect all clips in the timeline
void DeselectAllClips();

/// Dump the timeline: the sequence's contents and some parameters of the timeline (positions)
void DumpTimeline();

} // namespace

#endif // HELPER_TIMELINE_H