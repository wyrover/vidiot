#ifndef HELPER_TIMELINE_H
#define HELPER_TIMELINE_H

#include "UtilInt.h"
#include <boost/shared_ptr.hpp>
#include <set>
#include <wx/gdicmn.h>

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

typedef std::set<model::IClipPtr> DraggedClips; // todo rename to Dragged object and add comment indicating 'syntactic sugar to distinguish in the position lookup methods'
DraggedClips DragObject();

int getNonEmptyClipsCount();                ///< \return number of non-empty clips (both audio and video) in given timeline
int getSelectedClipsCount();                ///< \return number of selected clips (both audio and video) in given timeline

pixel LeftPixel(model::IClipPtr clip);          ///< \return left x position of given clip. Note: Returns the leftmost pixel value in the timeline that corresponds to the given clip.
pixel RightPixel(model::IClipPtr clip);         ///< \return right x position of given clip. Note: Returns the rightmost pixel value in the timeline that corresponds to the given clip.
pixel TopPixel(model::IClipPtr clip);           ///< \return top y position of given clip
pixel BottomPixel(model::IClipPtr clip);        ///< \return bottom y position of given clip

// todo replace with templated methods that use these four methods as input. THen drag + clip can be made generic
// then also split this file into helpertimelinegetters and helpertimelineactions
pixel VCenter(model::IClipPtr clip);            ///< \return vertical center position of given clip
pixel VQuarter(model::IClipPtr clip);           ///< \return y position at one quarter from the top
pixel HCenter(model::IClipPtr clip);            ///< \return horizontal center position of given clip
wxPoint Center(model::IClipPtr clip);           ///< \return center (pixel) position of a clip
wxPoint VQuarterHCenter(model::IClipPtr clip);  ///< \return pixel at one quarter from the top (centered horizontally)
wxPoint LeftCenter(model::IClipPtr clip);       ///< \return left center position (centered vertically)
wxPoint RightCenter(model::IClipPtr clip);      ///< \return right center position (centered vertically)

pixel LeftPixel(DraggedClips drag);             ///< \return left x position of the currently dragged clips (absolute position in the timeline, not relative to the drag offset)
pixel RightPixel(DraggedClips drag);            ///< \return right x position of the currently dragged clips (absolute position in the timeline, not relative to the drag offset)
pixel TopPixel(DraggedClips drag);              ///< \return top y position of the currently dragged clips (absolute position in the timeline, not relative to the drag offset)
pixel BottomPixel(DraggedClips drag);           ///< \return bottom y position of the currently dragged clips (absolute position in the timeline, not relative to the drag offset)

pixel VCenter(DraggedClips drag);               ///< \return vertical center position of the currently dragged clips (absolute position in the timeline, not relative to the drag offset)
pixel HCenter(DraggedClips drag);               ///< \return horizontal center position of given clip (absolute position in the timeline, not relative to the drag offset)
wxPoint Center(DraggedClips drag);              ///< \return center (pixel) position of the currently dragged clips (absolute position in the timeline, not relative to the drag offset)
wxPoint LeftCenter(DraggedClips drag);          ///< \return left center position (centered vertically) (absolute position in the timeline, not relative to the drag offset)
wxPoint RightCenter(DraggedClips drag);         ///< \return right center position (centered vertically) (absolute position in the timeline, not relative to the drag offset)

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

/// Do a drag from the current position until the left position of the dragged object is exactly on top of 'align'
/// \param align point to which the left position of the drag must be aligned
/// \note
/// The current LeftPixel(DraggedClips()) may be slightly different than the requested position.
/// When snapping is enabled, the snapping algorithm may cause a difference. If snapping is not
/// enabled, the current zoom factor may cause differences. In fact, when snapping is not enabled
/// not all pts values may be possible to 'drop on', since there's no matching pixel value. For
/// instance when zoomed with a factor 5 (each pixel corresponds to 5 pts values), then only the
/// pts values 1, and 5 are possible.
void DragAlignLeft(pixel position);
void DragAlignLeft(wxPoint from, pixel position, bool shift = false);

void ShiftDragAlignLeft(wxPoint from, pixel position);

void Scrub(pixel from, pixel to);                                       ///< Scrub the cursor over the timeline (view frames in preview window)
void Play(pixel from, int ms);                                          ///< Play from the given position, with the given time.

gui::timeline::MouseOnClipPosition LogicalPosition(wxPoint position);   ///< \return logical mouse position of the given (pixel) position.

/// Assert that there are no more transitions in the given track
void ASSERT_NO_TRANSITIONS_IN_VIDEO_TRACK(int trackindex = 0);

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