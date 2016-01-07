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

#include "TestTitles.h"
#include <boost/tuple/tuple_comparison.hpp>

namespace test {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

    void TestTitles::setUp()
{
    mProjectFixture.init();
}

    void TestTitles::tearDown()
{
    mProjectFixture.destroy();
}


//////////////////////////////////////////////////////////////////////////
// TEST CASES
//////////////////////////////////////////////////////////////////////////

void TestTitles::testFileTypes()
{
    StartTestSuite();

    // Find input files in dir (must be done after creating a project, due to dependencies on project properties for opening/closing files)
    wxFileName TestFilesPath = getTestPath();
    TestFilesPath.AppendDir("filetypes_title");
    ASSERT(TestFilesPath.IsDir());
    ASSERT(TestFilesPath.DirExists());
    model::IPaths InputFiles = GetSupportedFiles(TestFilesPath);

    model::SequencePtr sequence = getSequence();

    auto getPixel = [&](int x, int y) -> boost::tuple<int, int, int>
    {
        TimelinePositionCursor(0);
        return util::thread::RunInMainReturning<boost::tuple<int, int, int>>([sequence, x, y]() -> boost::tuple<int, int, int>
        {
            model::VideoFramePtr referenceFrame = sequence->getNextVideo(model::VideoCompositionParameters().setDrawBoundingBox(false).setBoundingBox(wxSize(model::Properties::get().getVideoSize())));
            wxImagePtr image = referenceFrame->getImage();
            return boost::make_tuple(image->GetRed(x, y), image->GetGreen(x, y), image->GetBlue(x, y));
        });
    };

    boost::tuple<int, int, int> referencePixel = getPixel(400, 400);

    WindowTriggerMenu(ID_ADDVIDEOTRACK);
    model::TrackPtr track = VideoTrack(1);
    WindowTriggerMenu(ID_CLOSESEQUENCE);

    for (model::IPathPtr path : InputFiles)
    {
        StartTest(path->getPath().GetFullName());
        model::FilePtr file = boost::make_shared<model::File>(path->getPath());
        ExtendTrack(track, { path }); // Note: Not via a command (thus, 'outside' the undo system)
        ProjectViewOpenTimelineForSequence(sequence);
        boost::tuple<int, int, int> curPixel = getPixel(400, 400);
        ASSERT_EQUALS(curPixel, referencePixel); // This pixel is taken from video layer 0 (the video)
        ASSERT_EQUALS(getPixel(0, 0), boost::make_tuple(0, 0, 0)); // This pixel is taken from the title
        ASSERT_EQUALS(NumberOfVideoClipsInTrack(1), 1);
        Play(HCenter(VideoClip(0, 0)), 400);
        WindowTriggerMenu(ID_CLOSESEQUENCE);

        MakeTrackEmpty(track); // Note: Not via a command (thus, 'outside' the undo system)
    }
}

} // namespace
