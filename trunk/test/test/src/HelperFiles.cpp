// Copyright 2014-2015 Eric Raijmakers.
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

#include "Test.h"

namespace test {

void ExecuteOnAllFiles(wxString pathToFiles, boost::function<void()> action, bool wait)
{
    // Create project (must be done after ConfigOverrule* code)
    model::FolderPtr root = WindowCreateProject();
    ASSERT(root);
    wxString sSequence( "Sequence" );
    model::SequencePtr sequence = ProjectViewAddSequence( sSequence, root );
    WindowTriggerMenu(ID_CLOSESEQUENCE);

    // Find input files in dir (must be done after creating a project, due to dependencies on project properties for opening/closing files)
    wxFileName TestFilesPath = getTestPath(); 
    TestFilesPath.AppendDir(pathToFiles);
    ASSERT(TestFilesPath.IsDir());
    ASSERT(TestFilesPath.DirExists());
    model::IPaths InputFiles = GetSupportedFiles(TestFilesPath);

    for ( model::IPathPtr path : InputFiles )
    {
        StartTest(path->getPath().GetFullName());
        model::FilePtr file = boost::make_shared<model::File>(path->getPath());

        ExtendSequenceWithRepeatedClips(sequence, { path }, 1); // Note: Not via a command (thus, 'outside' the undo system)
        
        ExpectExecutedWork expectation(wait ? 1 : 0, true);
        ProjectViewOpenTimelineForSequence(sequence);
        // Wait until audio peaks generated. Otherwise, not all save files have the same contents.
        // The later save files may have more entries in the meta data cache.
        if (wait)
        {
            // Ensure that audio peaks are generated (clip views large enough to hold the preview)
            TimelineZoomIn(4);

        }
        // Wait for audio peaks to be generated. For the longer audio files this results in reading through the entire file.
        // For instance, for Dawn_AnotherDay_EmbeddedCoverImage_IncompleteEndPacket.mp3 this caused an error when reading
        // the last packet of the file.
        expectation.wait();
        
        ASSERT_EQUALS(NumberOfVideoClipsInTrack(0),1);
        ASSERT_EQUALS(NumberOfAudioClipsInTrack(0),1);
        ASSERT_EQUALS(VideoTrack(0)->getLength(),AudioTrack(0)->getLength());
        action();
        WindowTriggerMenu(ID_CLOSESEQUENCE);
        MakeSequenceEmpty(sequence); // Note: Not via a command (thus, 'outside' the undo system)
    }
}

} // namespace
