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

#include "Timeline.h"

namespace gui { namespace timeline {
    enum MouseOnClipPosition : int;
}}

namespace test {

/// \return the physical position of the active timeline on the screen
wxPoint TimelinePosition();

int NumberOfVideoTracks();
int NumberOfAudioTracks();

/// Return the number of clips in a given video track
/// \param trackindex index position (0-based) of the video track, counting from the divider upwards
int NumberOfVideoClipsInTrack(int trackindex = 0);

/// Return the number of clips in a given audio track
/// \param trackindex index position (0-based) of the audio track, counting from the divider downwards
int NumberOfAudioClipsInTrack(int trackindex = 0);

/// Return a video track
/// \param trackindex index position (0-based) of the video track, counting from the divider upwards
model::VideoTrackPtr VideoTrack(int trackindex = 0);

/// Return a audio track
/// \param trackindex index position (0-based) of the audio track, counting from the divider downwards
model::AudioTrackPtr AudioTrack(int trackindex = 0);

/// Return a clip in a video track
/// \param trackindex index position (0-based) of the video track, counting from the divider upwards
/// \param clipindex index position (0-based) of the clip in the track, counting from left to right; if index < 0 then the counting is done 'from the right', where -1 is the last clip in the track.
model::IClipPtr VideoClip(int trackindex = 0, int clipindex = 0);

/// Return a video transition in a video track
/// \param trackindex index position (0-based) of the video track, counting from the divider upwards
/// \param clipindex index position (0-based) of the clip in the track, counting from left to right if index < 0 then the counting is done 'from the right', where -1 is the last clip in the track.
/// \pre the clip at the given position is a transitin
model::VideoTransitionPtr VideoTransition(int trackindex, int clipindex);

/// Return a clip in an audio track
/// \param trackindex index position (0-based) of the audio track, counting from the divider upwards
/// \param clipindex index position (0-based) of the clip in the track, counting from left to right
model::IClipPtr AudioClip(int trackindex = 0, int clipindex = 0);

int getNonEmptyClipsCount();     ///< \return number of non-empty clips (both audio and video) in given timeline
int getSelectedClipsCount(); ///< \return number of selected clips (both audio and video) in given timeline

pixel VCenter(model::TrackPtr track);

pixel RightPixel(model::TrackPtr track);

pixel TopPixel(model::TrackPtr track);

pixel VBottomQuarter(model::TrackPtr clip);

pixel LeftPixel(model::IClipPtr clip);

pixel RightPixel(model::IClipPtr clip);

pixel TopPixel(model::IClipPtr clip);

pixel BottomPixel(model::IClipPtr clip);

pixel VCenter(model::IClipPtr clip);

pixel VTopQuarter(model::IClipPtr clip);

pixel VBottomQuarter(model::IClipPtr clip);

pixel HCenter(model::IClipPtr clip);

wxPoint Center(model::IClipPtr clip);

wxPoint VTopQuarterHCenter(model::IClipPtr clip);

wxPoint VTopQuarterLeft(model::IClipPtr clip);

wxPoint VTopQuarterRight(model::IClipPtr clip);

wxPoint VBottomQuarterHCenter(model::IClipPtr clip);

wxPoint UnderTransitionLeftEdge(model::IClipPtr clip);

wxPoint UnderTransitionRightEdge(model::IClipPtr clip);

wxPoint LeftCenter(model::IClipPtr clip);

wxPoint RightCenter(model::IClipPtr clip);

wxPoint LeftBottom(model::IClipPtr clip);

wxPoint RightBottom(model::IClipPtr clip);

wxPoint OnTimescaleAbove(wxPoint position);

class DraggedClips {};                               ///< Syntactic sugar to be able to use the same syntax (as used for determinig clip positions) for determining the position of the current drag object

pixel LeftPixel(DraggedClips drag);                  ///< \return left x position of the currently dragged clips (absolute position in the timeline, not relative to the drag offset)
pixel RightPixel(DraggedClips drag);                 ///< \return right x position of the currently dragged clips (absolute position in the timeline, not relative to the drag offset)
pixel TopPixel(DraggedClips drag);                   ///< \return top y position of the currently dragged clips (absolute position in the timeline, not relative to the drag offset)
pixel BottomPixel(DraggedClips drag);                ///< \return bottom y position of the currently dragged clips (absolute position in the timeline, not relative to the drag offset)

pixel VCenter(DraggedClips drag);                    ///< \return vertical center position of the currently dragged clips (absolute position in the timeline, not relative to the drag offset)
pixel HCenter(DraggedClips drag);                    ///< \return horizontal center position of given clip (absolute position in the timeline, not relative to the drag offset)
wxPoint Center(DraggedClips drag);                   ///< \return center (pixel) position of the currently dragged clips (absolute position in the timeline, not relative to the drag offset)
wxPoint LeftCenter(DraggedClips drag);               ///< \return left center position (centered vertically) (absolute position in the timeline, not relative to the drag offset)
wxPoint RightCenter(DraggedClips drag);              ///< \return right center position (centered vertically) (absolute position in the timeline, not relative to the drag offset)

/// Trigger a wheel event on the timeline
/// \nSteps number of wheel steps to trigger (+1 up, -1 down)
void TimelineTriggerWheel(int nSteps);

pixel CursorPosition();                              ///< \return cursor position

void TimelinePositionCursor(pixel position);                 ///< Move the mouse and then click the left button, in order to move the cursor line to the given position

/// Zoom in 
/// \param level number of steps to zoom in
void TimelineZoomIn(int level); 

/// Zoom out
/// \param level number of steps to zoom in
void TimelineZoomOut(int level); 

void ToggleInterval(pixel from, pixel to);

void Scrub(pixel from, pixel to);                                       ///< Scrub the cursor over the timeline (view frames in preview window)
void ScrollWithRightMouseButton(pixel distance);                        ///< Scroll the timeline using right mouse scrolling

gui::timeline::MouseOnClipPosition LogicalPosition(wxPoint position);   ///< \return logical mouse position of the given (pixel) position.

/// Select the given clips (and their links, if any) by ctrl-clicking all of them.
/// Selection is made empty beforehand.
void TimelineSelectClips(model::IClips clips);

/// Delete the given clip (and its link, if any) from the timeline
/// \param clip clip to be deleted from the timeline
/// \param shift if true then shift-delete is pressed (shift the other clips into the moved area).
void TimelineDeleteClip(model::IClipPtr clip, bool shift = false);

/// Delete the given clips (and their links, if any) from the timeline
/// \param clips clips to be deleted from the timeline
/// \param shift if true then shift-delete is pressed (shift the other clips into the moved area).
void TimelineDeleteClips(model::IClips clips, bool shift = false);

/// Delete the given clip (and its link, if any) from the timeline via shift-delete
/// \param clip clip to be deleted from the timeline
void TimelineShiftDeleteClip(model::IClipPtr clip);

/// Delete the given clips (and their links, if any) from the timeline via shift-delete
/// \param clips clips to be deleted from the timeline
void TimelineShiftDeleteClips(model::IClips clips);

/// Dump the current sequence.
void DumpSequence();

/// Dump the current sequence.
/// Then wait forever. That allows undoing/redoing/further tampering for analysis.
void DumpSequenceAndWait();

//////////////////////////////////////////////////////////////////////////
// WAITFORTIMELINETOLOSEFOCUS
//////////////////////////////////////////////////////////////////////////

class WaitForTimelineToLoseFocus
{
public:
    WaitForTimelineToLoseFocus();
    ~WaitForTimelineToLoseFocus();
    void wait();
private:
    void onLeave(wxMouseEvent& event);
    void onFocus(wxFocusEvent& event);
    void endWait();
    bool mFound;
    boost::condition_variable mCondition;
    boost::mutex mMutex;
};

} // namespace
