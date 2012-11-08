#ifndef HELPER_TIMELINE_H
#define HELPER_TIMELINE_H

#include "UtilInt.h"
#include <boost/shared_ptr.hpp>
#include <set>
#include <wx/gdicmn.h>
#include "Timeline.h"

namespace model {
class IClip;
typedef boost::shared_ptr<IClip> IClipPtr;
class VideoTrack;
typedef boost::shared_ptr<VideoTrack> VideoTrackPtr;
class VideoTransition;
typedef boost::shared_ptr<VideoTransition> VideoTransitionPtr;
class AudioTrack;
typedef boost::shared_ptr<AudioTrack> AudioTrackPtr;
class Track;
typedef boost::shared_ptr<Track> TrackPtr;
}

namespace gui { namespace timeline {
    enum MouseOnClipPosition;
}}

namespace test {

/// \return the physical position of the active timeline on the screen
wxPoint TimelinePosition();

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

/// \return vertical center pixel of given track within the sequence view
/// \note This does take into account zooming (which is part of the sequence view), but does NOT take into account scrolling.
pixel VCenter(model::TrackPtr track);

/// \return left x position of given clip within the sequence view. Note: Returns the leftmost pixel value in the timeline that corresponds to the given clip.
/// \note This does take into account zooming (which is part of the sequence view), but does NOT take into account scrolling.
pixel LeftPixel(model::IClipPtr clip);

/// \return right x position of given clip within the sequence view. Note: Returns the rightmost pixel value in the timeline that corresponds to the given clip.
/// \note This does take into account zooming (which is part of the sequence view), but does NOT take into account scrolling.
pixel RightPixel(model::IClipPtr clip);

/// \return top y position of given clip within the sequence view.
/// \note This does take into account zooming (which is part of the sequence view), but does NOT take into account scrolling.
pixel TopPixel(model::IClipPtr clip);

/// \return bottom y position of given clip within the sequence view.
/// \note This does take into account zooming (which is part of the sequence view), but does NOT take into account scrolling.
pixel BottomPixel(model::IClipPtr clip);

/// \return vertical center position of given clip within the sequence view.
/// \note This does take into account zooming (which is part of the sequence view), but does NOT take into account scrolling.
pixel VCenter(model::IClipPtr clip);

/// \return y position at one quarter from the top within the sequence view.
/// \note This does take into account zooming (which is part of the sequence view), but does NOT take into account scrolling.
pixel VTopQuarter(model::IClipPtr clip);

/// \return y position at one quarter from the bottom within the sequence view.
/// \note This does take into account zooming (which is part of the sequence view), but does NOT take into account scrolling.
pixel VBottomQuarter(model::IClipPtr clip);

/// \return horizontal center position of given clip within the sequence view.
/// \note This does take into account zooming (which is part of the sequence view), but does NOT take into account scrolling.
pixel HCenter(model::IClipPtr clip);

/// \return center (pixel) position of a clip within the sequence view.
/// \note This does take into account zooming (which is part of the sequence view), but does NOT take into account scrolling.
wxPoint Center(model::IClipPtr clip);

/// \return pixel at one quarter from the top (centered horizontally) within the sequence view.
/// \note This does take into account zooming (which is part of the sequence view), but does NOT take into account scrolling.
wxPoint VTopQuarterHCenter(model::IClipPtr clip);

/// \return pixel at one quarter from the top, left edge within the sequence view.
/// \note This does take into account zooming (which is part of the sequence view), but does NOT take into account scrolling.
wxPoint VTopQuarterLeft(model::IClipPtr clip);

/// \return pixel at one quarter from the top, right edge within the sequence view.
/// \note This does take into account zooming (which is part of the sequence view), but does NOT take into account scrolling.
wxPoint VTopQuarterRight(model::IClipPtr clip);

/// \return pixel at one quarter from the bottom (centered horizontally) within the sequence view.
/// \note This does take into account zooming (which is part of the sequence view), but does NOT take into account scrolling.
wxPoint VBottomQuarterHCenter(model::IClipPtr clip);

/// \return pixel at one quarter from the bottom (horizontally left) within the sequence view.
/// \note This does take into account zooming (which is part of the sequence view), but does NOT take into account scrolling.
wxPoint LeftVBottomQuarter(model::IClipPtr clip);

/// \return pixel at one quarter from the bottom (horizontally right) within the sequence view.
/// \note This does take into account zooming (which is part of the sequence view), but does NOT take into account scrolling.
wxPoint RightVBottomQuarter(model::IClipPtr clip);

/// \return left center position (centered vertically) within the sequence view.
/// \note This does take into account zooming (which is part of the sequence view), but does NOT take into account scrolling.
wxPoint LeftCenter(model::IClipPtr clip);

/// \return right center position (centered vertically) within the sequence view.
/// \note This does take into account zooming (which is part of the sequence view), but does NOT take into account scrolling.
wxPoint RightCenter(model::IClipPtr clip);

/// \return left bottom position within the sequence view.
/// \note This does take into account zooming (which is part of the sequence view), but does NOT take into account scrolling.
wxPoint LeftBottom(model::IClipPtr clip);

/// \return right bottom position within the sequence view.
/// \note This does take into account zooming (which is part of the sequence view), but does NOT take into account scrolling.
wxPoint RightBottom(model::IClipPtr clip);

/// \return position adjusted such that the y-position is on the time scale above the clips within the sequence view.
/// \note This does NOT take into account scrolling.
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

pixel CursorPosition();                              ///< \return cursor position

void PositionCursor(pixel position);                                    ///< Move the mouse and then click the left button, in order to move the cursor line to the given position
void Move(wxPoint position);                                            ///< Move the mouse to the given position within the timeline
void Click(wxPoint position);                                           ///< Move the mouse to the given position and (left) click there
void TrimLeft(model::IClipPtr clip, pixel length, bool shift = true, bool endtrim = true);   ///< Trim the given clip on the left side
void TrimRight(model::IClipPtr clip, pixel length, bool shift = true, bool endtrim = true);  ///< Trim the given clip on the right side
void BeginTrim(wxPoint from, bool shift);
void EndTrim(bool shift = true);

/// Zoom in the given amount of times by zooming in the given amount of times.
/// When going out of scope, will reset to the default zoom level by zooming out an equal amount of times.
struct Zoom
{
    explicit Zoom(int level);
    ~Zoom();
private:
    int mLevel;
};

/// Do a trim between the two points (press, move, release). This basically does the same as 'Drag' but faster. The Drag
/// method does the move in several (10) steps. This method simply moves to the begin point, presses the mouse, moves
/// to the end point (without intermediate points) and releases the button.
/// \param from starting position to move to initially
/// \param to final position to drag to
/// \param shift hold down shift after pressing the mouse button
void Trim(wxPoint from, wxPoint to, bool shift = false);

/// \see Trim
/// Do a shift trim
void ShiftTrim(wxPoint from, wxPoint to);

/// Do a drag and drop between the two points (press, move, release).
/// \param from starting position to move to initially
/// \param to final position to drag to
/// \param ctrl if true, then ctrl is pressed at the beginning of the drag and released directly after moving 'a bit'
/// \param mousedown if true, then the mouse button is pressed just after moving to 'from' and before initiating the drag
/// \param mouseup if true, then the mouse button is released after the mouse has reached position 'to'
void Drag(wxPoint from, wxPoint to, bool ctrl = false, bool mousedown = true, bool mouseup = true);

/// Press ctrl, move to 'from', then start dragging, release ctrl, and then drop on point 'to'
/// \param from starting position to move to initially
/// \param to final position to drag to
void CtrlDrag(wxPoint from, wxPoint to, bool mouseup = true);

/// Do a shift drag and drop between the two points (press, move a bit, hold shift, move to 'to', release mouse and shift).
/// \param from starting position to move to initially
/// \param to final position to drag to
void ShiftDrag(wxPoint from, wxPoint to);

/// Do a drag from the current position until the left position of the dragged object is exactly on top of 'position'
/// \param position point to which the left position of the drag must be aligned
/// \param from starting position of the drag operation (the mouse will be pressed here)
/// Shift is not pressed during the drag operation.
/// \note
/// The current LeftPixel(DraggedClips()) may be slightly different than the requested position.
/// When snapping is enabled, the snapping algorithm may cause a difference. If snapping is not
/// enabled, the current zoom factor may cause differences. In fact, when snapping is not enabled
/// not all pts values may be possible to 'drop on', since there's no matching pixel value. For
/// instance when zoomed with a factor 5 (each pixel corresponds to 5 pts values), then only the
/// pts values 1, and 5 are possible.
void DragAlignLeft(wxPoint from, pixel position);

/// \see DragAlignLeft
/// Do a drag from the current position until the left position of the dragged object is exactly on top of 'align'
/// Shift is pressed during the drag operation causing a shift-drag (insert clips instead of overwrite)
void ShiftDragAlignLeft(wxPoint from, pixel position);

/// Similar to DragAlignLeft with the exception that the right position of the dragged object is aligned to the given position.
/// \see DragAlignLeft
void DragAlignRight(wxPoint from, pixel position);

/// Similar to ShiftDragAlignLeft with the exception that the right position of the dragged object is aligned to the given position.
/// \see ShiftDragAlignLeft
void ShiftDragAlignRight(wxPoint from, pixel position);

/// Drag videoclip to VideoTrack(newtrackindex) and
/// Drag audioclip to AudioTrack(newtrackindex)
/// \param videoclip video clip to be moved to the video track with index newtrackindex
/// \param audioclip audio clip to be moved to the audio track with index newtrackindex
/// \param newtrackindex new track of clip
void DragToTrack(int newtrackindex, model::IClipPtr videoclip, model::IClipPtr audioclip);

void ToggleInterval(pixel from, pixel to);

void Scrub(pixel from, pixel to);                                       ///< Scrub the cursor over the timeline (view frames in preview window)
void Play(pixel from, int ms);                                          ///< Play from the given position, with the given time.

gui::timeline::MouseOnClipPosition LogicalPosition(wxPoint position);   ///< \return logical mouse position of the given (pixel) position.

/// Unselect all clips in the timeline
void DeselectAllClips();

/// Delete the given clip from the timeline
/// \param clip clip to be deleted from the timeline
void DeleteClip(model::IClipPtr clip);

/// Dump the current sequence.
/// Then wait forever. That allows undoing/redoing/further tampering for analysis.
void DumpSequenceAndWait();

} // namespace

#endif // HELPER_TIMELINE_H