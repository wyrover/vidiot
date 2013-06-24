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

/// \return rightmost pixel of given Track within the sequence view
/// \note This does take into account zooming (which is part of the sequence view), but does NOT take into account scrolling.
pixel RightPixel(model::TrackPtr track);

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

/// Zoom in the given amount of times by zooming in the given amount of times.
/// When going out of scope, will reset to the default zoom level by zooming out an equal amount of times.
struct Zoom
{
    explicit Zoom(int level);
    ~Zoom();
private:
    int mLevel;
};

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
void DumpSequence();

/// Dump the current sequence.
/// Then wait forever. That allows undoing/redoing/further tampering for analysis.
void DumpSequenceAndWait();

} // namespace

#endif // HELPER_TIMELINE_H