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

#include "TestBugsNoProject.h"

#include "UtilInitAvcodec.h"

namespace test {

//////////////////////////////////////////////////////////////////////////
// TEST CASES
//////////////////////////////////////////////////////////////////////////

void TestBugsNoProject::testCrashWhenOneClipIsTooLarge()
{
    StartTestSuite();
    model::SequencePtr sequence{ CreateProjectWithClosedSequence() };
    util::thread::RunInMainAndWait([sequence]()
    {
        ASSERT(sequence);
        model::TrackPtr videoTrack{ sequence->getVideoTrack(0) };
        ASSERT(videoTrack);
        wxFileName filename{ getTestFilesPath("filetypes_image") };
        filename.SetFullName("Laney -6th best amp.png");
        model::FilePtr file{ boost::make_shared<model::File>(filename) };
        model::VideoClipPtr videoclip = boost::dynamic_pointer_cast<model::VideoClip>(boost::make_shared<model::WximageClip>(boost::make_shared<model::WximageFile>(file->getPath())));
        videoclip->adjustEnd(25 * 3600 * 6); // 6 hour clip...
        videoTrack->addClips({ videoclip });
    });
    ProjectViewOpenTimelineForSequence(sequence);
    Play(LeftPixel(VideoClip(0,0)), 150);
    TimelinePositionCursor(HCenter(VideoClip(0, 0)));
    TimelineKeyPress('v'); // Show the part being played (for longer files)
    Play(HCenter(VideoClip(0, 0)), 150);
    TimelinePositionCursor(RightPixel(VideoClip(0, 0)) - 25);
    TimelineKeyPress('v'); // Show the part being played (for longer files)
    Play(RightPixel(VideoClip(0, 0)) - 25, 150);
}

} // namespace
