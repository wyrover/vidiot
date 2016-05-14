// Copyright 2014-2016 Eric Raijmakers.
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

#include "TestUiBugs.h"
//todo make testui work on linux
namespace test {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

void TestUiBugs::setUp()
{
    mProjectFixture.init();
}

void TestUiBugs::tearDown()
{
    mProjectFixture.destroy();
}

//////////////////////////////////////////////////////////////////////////
// TEST CASES
//////////////////////////////////////////////////////////////////////////

void TestUiBugs::testDragAndDropVideoFileLargerThanTimeline()
{
    StartTestSuite();
    TimelineZoomIn(2);

    model::Files files = ProjectViewAddFiles({ SpecialFile("long_black_10min.avi") });

    DragFromProjectViewToTimeline(files.front(), getTimeline().GetScreenPosition() + wxPoint(5, VCenter(VideoTrack(0))));
    ASSERT_EQUALS(NumberOfVideoClipsInTrack(0), 1); // Dropped clip obscured entire track
    ASSERT_EQUALS(NumberOfAudioClipsInTrack(0), 7);
    ASSERT_MORE_THAN_EQUALS(VideoClip(0)->getLength(), model::Convert::timeToPts(10 * 60 * 1000));
}

void TestUiBugs::testDragAndDropAudioFileLargerThanTimeline()
{
    StartTestSuite();
    TimelineZoomIn(2);

    model::Files files = ProjectViewAddFiles({ SpecialFile("Dawn - Another Day.mp3") });

    DragFromProjectViewToTimeline(files.front(), getTimeline().GetScreenPosition() + wxPoint(5, VCenter(AudioTrack(0))));
    ASSERT_EQUALS(NumberOfVideoClipsInTrack(0), 7);
    ASSERT_EQUALS(NumberOfAudioClipsInTrack(0), 1); // Dropped clip obscured entire track
    ASSERT_MORE_THAN_EQUALS(AudioClip(0)->getLength(), model::Convert::timeToPts(3 * 60 * 1000 + 27 * 1000));
}

void TestUiBugs::testCrashWhenRightClickingOutsideAllTracks()
{
    StartTestSuite();
    TimelineRightClick(wxPoint(200, gui::timeline::TimescaleView::TimeScaleHeight + 2));
    TimelineKeyPress(WXK_ESCAPE);
}

void TestUiBugs::testDragAndDropWithAddingTracksWithSelectedClipInTopMostTrackForVideo()
{
    StartTestSuite();

    wxPoint PositionForTriggeringAddTrack{ getTimeline().GetScreenPosition() + wxPoint(50, gui::timeline::TimescaleView::TimeScaleHeight + 4) };
    wxPoint PositionInsideAddedTrack{ getTimeline().GetScreenPosition() + wxPoint(50, gui::timeline::TimescaleView::TimeScaleHeight + 25) };

    model::NodePtr node = ProjectViewFind("01.avi");
    ProjectViewExpandInput();
    DragFromProjectViewToTimeline(node, PositionForTriggeringAddTrack);
    ASSERT_VIDEOTRACKS(2);
    ASSERT_VIDEOTRACK1(EmptyClip)(VideoClip);
    DragFromProjectViewToTimeline(node, PositionForTriggeringAddTrack);
    ASSERT_VIDEOTRACKS(3);
    ASSERT_VIDEOTRACK2(EmptyClip)(VideoClip);
    TimelineSelectClips({ VideoClip(2, 1) });
    // Dragged further: When the mouse pointer was inside the newly created track during the DND,
    // the drag feedback was flickering (visible/invisible continously) and the drop caused an error when deleting all clips.
    DragFromProjectViewToTimeline(node, PositionInsideAddedTrack, { PositionForTriggeringAddTrack });

    TimelineLeftClick(wxPoint{ 5,5 }); // Give focus
    TimelineKeyPress(WXK_CONTROL, 'a'); // Select all clips
    TimelineKeyPress(WXK_DELETE);  // Delete all clips (including the clip that caused the crash when being deleted)

    ASSERT_VIDEOTRACKS(4);
    ASSERT_VIDEOTRACK0SIZE(0);
    ASSERT_VIDEOTRACK1SIZE(0);
    ASSERT_VIDEOTRACK2SIZE(0);
    ASSERT_VIDEOTRACK3SIZE(0);
}

void TestUiBugs::testDragAndDropWithAddingTracksWithSelectedClipInTopMostTrackForVideoOnly()
{
    StartTestSuite();

    wxPoint PositionForTriggeringAddTrack{ getTimeline().GetScreenPosition() + wxPoint(50, gui::timeline::TimescaleView::TimeScaleHeight + 4) };
    wxPoint PositionInsideAddedTrack{ getTimeline().GetScreenPosition() + wxPoint(50, gui::timeline::TimescaleView::TimeScaleHeight + 25) };

    model::NodePtr node = ProjectViewAddFiles({ SpecialFile("1_minute_black_video_only.avi") }).front(); // Video only
    ProjectViewExpandInput();
    DragFromProjectViewToTimeline(node, PositionForTriggeringAddTrack);
    ASSERT_VIDEOTRACKS(2);
    ASSERT_VIDEOTRACK1(VideoClip);
    DragFromProjectViewToTimeline(node, PositionForTriggeringAddTrack);
    ASSERT_VIDEOTRACKS(3);
    ASSERT_VIDEOTRACK2(VideoClip);
    TimelineSelectClips({ VideoClip(2, 0) });
    // Dragged further: When the mouse pointer was inside the newly created track during the DND,
    // the drag feedback was flickering (visible/invisible continously) and the drop caused an error when deleting all clips.
    DragFromProjectViewToTimeline(node, PositionInsideAddedTrack, { PositionForTriggeringAddTrack });

    TimelineLeftClick(wxPoint{ 5,5 }); // Give focus
    TimelineKeyPress(WXK_CONTROL, 'a'); // Select all clips
    TimelineKeyPress(WXK_DELETE);  // Delete all clips (including the clip that caused the crash when being deleted)

    ASSERT_VIDEOTRACKS(4);
    ASSERT_VIDEOTRACK0SIZE(0);
    ASSERT_VIDEOTRACK1SIZE(0);
    ASSERT_VIDEOTRACK2SIZE(0);
    ASSERT_VIDEOTRACK3SIZE(0);
}

void TestUiBugs::testDragAndDropWithAddingTracksWithSelectedClipInTopMostTrackForAudio()
{
    StartTestSuite();

    model::NodePtr node = ProjectViewFind("01.avi");
    ProjectViewExpandInput();

    wxPoint PositionForTriggeringAddTrack{ getTimeline().GetScreenPosition() + wxPoint(50, getTimeline().GetSize().GetHeight() - 5) };

    DragFromProjectViewToTimeline(node, PositionForTriggeringAddTrack);
    ASSERT_AUDIOTRACKS(2);
    ASSERT_AUDIOTRACK1(EmptyClip)(AudioClip);
    DragFromProjectViewToTimeline(node, PositionForTriggeringAddTrack);
    ASSERT_AUDIOTRACKS(3);
    ASSERT_AUDIOTRACK2(EmptyClip)(AudioClip);

    DragTrackDivider(VideoTrack(0), 30);
    DragAudioVideoDivider(-30);
    DragTrackDivider(AudioTrack(0), -30);
    DragTrackDivider(AudioTrack(1), -30);
    
    wxPoint PositionInsideAddedTrack{ getTimeline().GetScreenPosition() + wxPoint(50, BottomPixel(AudioTrack(2)) + gui::timeline::DividerView::TrackDividerHeight + (model::Track::sDefaultTrackHeight / 2)) };
    wxPoint PositionInsideTrackAboveAddedTrack{ getTimeline().GetScreenPosition() + wxPoint(50, VCenter(AudioTrack(2))) };

    TimelineSelectClips({ AudioClip(2, 1) }); 
    // Drag sequence: When the mouse pointer was moved inside the newly created track during the DND, from the track above it,
    // the drag feedback was flickering (visible/invisible continously) and the drop caused an error when deleting all clips.
    DragFromProjectViewToTimeline(node, PositionInsideAddedTrack, { PositionForTriggeringAddTrack, PositionInsideAddedTrack, PositionInsideTrackAboveAddedTrack });

    TimelineLeftClick(wxPoint{ 5,5 }); // Give focus
    TimelineKeyPress(WXK_CONTROL, 'a'); // Select all clips
    TimelineKeyPress(WXK_DELETE);  // Delete all clips (including the clip that caused the crash when being deleted)

    ASSERT_AUDIOTRACKS(4);
    ASSERT_AUDIOTRACK0SIZE(0);
    ASSERT_AUDIOTRACK1SIZE(0);
    ASSERT_AUDIOTRACK2SIZE(0);
    ASSERT_AUDIOTRACK3SIZE(0);
}

void TestUiBugs::testDragAndDropWithAddingTracksWithSelectedClipInTopMostTrackForAudioOnly()
{
    StartTestSuite();

    model::NodePtr node = ProjectViewAddFiles({ SpecialFile("30_seconds_silence_audio_only.wav") }).front();
    ProjectViewExpandInput();

    wxPoint PositionForTriggeringAddTrack{ getTimeline().GetScreenPosition() + wxPoint(50, getTimeline().GetSize().GetHeight() - 5) };

    DragFromProjectViewToTimeline(node, PositionForTriggeringAddTrack);
    ASSERT_AUDIOTRACKS(2);
    ASSERT_AUDIOTRACK1(AudioClip);
    DragFromProjectViewToTimeline(node, PositionForTriggeringAddTrack);
    ASSERT_AUDIOTRACKS(3);
    ASSERT_AUDIOTRACK2(AudioClip);

    DragTrackDivider(VideoTrack(0), 30);
    DragAudioVideoDivider(-30);
    DragTrackDivider(AudioTrack(0), -30);
    DragTrackDivider(AudioTrack(1), -30);

    wxPoint PositionInsideAddedTrack{ getTimeline().GetScreenPosition() + wxPoint(50, BottomPixel(AudioTrack(2)) + gui::timeline::DividerView::TrackDividerHeight + (model::Track::sDefaultTrackHeight / 2)) };
    wxPoint PositionInsideTrackAboveAddedTrack{ getTimeline().GetScreenPosition() + wxPoint(50, VCenter(AudioTrack(2))) };

    TimelineSelectClips({ AudioClip(2, 0) });
    // Drag sequence: When the mouse pointer was moved inside the newly created track during the DND, from the track above it,
    // the drag feedback was flickering (visible/invisible continously) and the drop caused an error when deleting all clips.
    DragFromProjectViewToTimeline(node, PositionInsideAddedTrack, { PositionForTriggeringAddTrack, PositionInsideAddedTrack, PositionInsideTrackAboveAddedTrack });

    TimelineLeftClick(wxPoint{ 5,5 }); // Give focus
    TimelineKeyPress(WXK_CONTROL, 'a'); // Select all clips
    TimelineKeyPress(WXK_DELETE);  // Delete all clips (including the clip that caused the crash when being deleted)

    ASSERT_AUDIOTRACKS(4);
    ASSERT_AUDIOTRACK0SIZE(0);
    ASSERT_AUDIOTRACK1SIZE(0);
    ASSERT_AUDIOTRACK2SIZE(0);
    ASSERT_AUDIOTRACK3SIZE(0);
}
} // namespace
