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

#include "HelperRender.h"

namespace test {

std::pair< RandomTempDirPtr, wxFileName > OpenRenderDialogAndSetFilename()
{
    WindowTriggerMenu(ID_RENDERSETTINGS);
    WaitUntilDialogOpen(true);

    RandomTempDirPtr tempdir = boost::make_shared<RandomTempDir>();
    wxFileName path(tempdir->getFileName().GetFullPath(), "out", "avi");
    gui::Dialog::get().setSaveFile(path.GetFullPath());
    ButtonTriggerPressed(gui::DialogRenderSettings::get().getFileButton());
    return std::make_pair(tempdir,path);
}

std::pair< RandomTempDirPtr, wxFileName > RenderTimeline(int lengthInS)
{
    ConfigOverrule<long> overrule(Config::sPathDebugMaxRenderLength, lengthInS);
    std::pair< RandomTempDirPtr, wxFileName > tempdir_and_filename = OpenRenderDialogAndSetFilename();
    ExpectExecutedWork expectation(1);
    ButtonTriggerPressed(gui::DialogRenderSettings::get().getRenderButton());
    WaitUntilMainWindowActive(true);
    expectation.wait();
    ASSERT(tempdir_and_filename.second.Exists());
    model::FilePtr file{ boost::make_shared<model::File>(tempdir_and_filename.second) };
    ASSERT_MORE_THAN_EQUALS(file->getLength(), model::Convert::timeToPts(lengthInS * 1000));
    return tempdir_and_filename;
}

void PlaybackRenderedTimeline(const wxFileName& path, pixel start, milliseconds t)
{
    model::FolderPtr folder1 = ProjectViewAddFolder( "PlaybackRenderedTimeline" );
    model::Files files1 = ProjectViewAddFiles({ path }, folder1);
    model::SequencePtr sequence1 = ProjectViewCreateSequence( folder1 );
    TimelineZoomIn(4);
    Play(start, t);
    ProjectViewRemove(sequence1);
    ProjectViewRemove(folder1);
}

void RenderAndPlaybackCurrentTimeline(int renderedlengthInS, pixel playbackStart, milliseconds playbackLength)
{
    RandomTempDir tempdir;
    std::pair< RandomTempDirPtr, wxFileName > tempdir_and_filename =  RenderTimeline(renderedlengthInS);
    WindowTriggerMenu(ID_CLOSESEQUENCE);
    PlaybackRenderedTimeline(tempdir_and_filename.second, playbackStart, playbackLength);
}

} // namespace
