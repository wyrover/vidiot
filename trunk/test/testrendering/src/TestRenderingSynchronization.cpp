// Copyright 2014 Eric Raijmakers.
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

#include "TestRenderingSynchronization.h"

namespace test {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

void TestRenderingSynchronization::setUp()
{
    mProjectFixture.init();
}

void TestRenderingSynchronization::tearDown()
{
    mProjectFixture.destroy();
}

//////////////////////////////////////////////////////////////////////////
// TEST CASES
//////////////////////////////////////////////////////////////////////////

void TestRenderingSynchronization::testLongSequence()
{
    StartTestSuite();

    const int EMPTY_LENGTH = 23 * 60 * 1000; // 23 minutes of empty
    model::SequencePtr sequence = getSequence();

    StartTest("Set codecs");

    WindowTriggerMenu(ID_RENDERSETTINGS);
    gui::DialogRenderSettings::get().getVideoCodecButton()->select(AV_CODEC_ID_MPEG4);
    gui::DialogRenderSettings::get().getAudioCodecButton()->select(AV_CODEC_ID_MP3);
    ButtonTriggerPressed(gui::DialogRenderSettings::get().getOkButton());
    WindowTriggerMenu(ID_CLOSESEQUENCE);

    StartTest("Create movie");
    wxFileName syncFileName = getTestPath(); 
    syncFileName.AppendDir("filetypes_sync");
    ASSERT(syncFileName.IsDir());
    ASSERT(syncFileName.DirExists());
    syncFileName.SetFullName("Audio Video Sync Test & Calibration 23976fps.xvid.avi");
    model::FilePtr syncFile = boost::make_shared<model::File>(syncFileName );
    MakeSequenceEmpty(sequence);
    ExtendSequenceWithRepeatedClips(sequence, boost::assign::list_of(syncFile), 1); // Note: Not via a command (thus, 'outside' the undo system)
    ExtendSequenceWithEmptyClipAtBegin(sequence, EMPTY_LENGTH);
    ProjectViewOpenTimelineForSequence(sequence);

    StartTest("Play before render");
    TimelinePositionCursor(getTimeline().getZoom().ptsToPixels(model::Convert::timeToPts(EMPTY_LENGTH - 1000))); // 1 second before the end of the empty region
    TimelineKeyPress('c'); // Show the proper timeline part
    Play(4000);

    StartTest("Render");
    std::pair< RandomTempDirPtr, wxFileName > tempdir_and_filename = RenderTimeline(0); // 0: Render all
    WindowTriggerMenu(ID_CLOSESEQUENCE);

    StartTest("Open");
    model::FolderPtr folder1 = ProjectViewAddFolder("Playback");
    model::Files files1 = ProjectViewAddFiles( boost::assign::list_of(tempdir_and_filename.second), folder1 );
    model::SequencePtr sequence1 = ProjectViewCreateSequence( folder1 );
    TimelinePositionCursor(getTimeline().getZoom().ptsToPixels(model::Convert::timeToPts(EMPTY_LENGTH - 1000))); // 1 second before the end of the empty region
    TimelineKeyPress('c'); // Show the proper timeline part

    StartTest("Play rendered");
    OpenFileExplorer(tempdir_and_filename.first->getFileName());
    gui::Dialog::get().getConfirmation("Play back movie", "Press ok play back the movie (note: also test playback the file on disk directly!)");
    Play(8000);
    WindowTriggerMenu(ID_CLOSESEQUENCE);
    SetProjectUnmodified();
    pause(1000); // Avoid problems with file still in use
    WindowTriggerMenu(wxID_CLOSE);
}

} // namespace