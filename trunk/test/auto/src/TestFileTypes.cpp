// Copyright 2013 Eric Raijmakers.
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

#include "TestFileTypes.h"

#include "AudioClip.h"
#include "AudioTrack.h"
#include "Config.h"
#include "EmptyClip.h"
#include "File.h"
#include "HelperConfig.h"
#include "HelperProjectView.h"
#include "HelperTimeline.h"
#include "HelperTimelineAssert.h"
#include "HelperTimelinesView.h"
#include "HelperWindow.h"
#include "IClip.h"
#include "IPath.h"
#include "Scrolling.h"
#include "Timeline.h"
#include "UtilFrameRate.h"
#include "VideoClip.h"
#include "VideoTrack.h"
#include <boost/assign/list_of.hpp>

namespace test {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

void TestFileTypes::setUp()
{
#ifndef SOURCE_ROOT
#error "SOURCE_ROOT is not defined!"
#endif
}

void TestFileTypes::tearDown()
{
}

//////////////////////////////////////////////////////////////////////////
// TEST CASES
//////////////////////////////////////////////////////////////////////////

void TestFileTypes::testFileTypes_audio_1_44100()
{
    StartTestSuite();
    ConfigOverruleLong overruleChannels(Config::sPathDefaultAudioChannels, 1);
    ConfigOverruleLong overruleSampleRate(Config::sPathDefaultAudioSampleRate, 44100);
    executeTest("filetypes_audio");
}

void TestFileTypes::testFileTypes_audio_2_48000()
{
    StartTestSuite();
    ConfigOverruleLong overruleChannels(Config::sPathDefaultAudioChannels, 2);
    ConfigOverruleLong overruleSampleRate(Config::sPathDefaultAudioSampleRate, 48000);
    executeTest("filetypes_audio");
}

void TestFileTypes::testFileTypes_image()
{
    StartTestSuite();
    executeTest("filetypes_image");
}

void TestFileTypes::testFileTypes_video_s24p()
{
    StartTestSuite();
    ConfigOverruleString overruleFrameRate(Config::sPathDefaultFrameRate, FrameRate::s24p.toString());
    executeTest("filetypes_video");
}

void TestFileTypes::testFileTypes_video_s25p()
{
    StartTestSuite();
    ConfigOverruleString overruleFrameRate(Config::sPathDefaultFrameRate, FrameRate::s25p.toString());
    executeTest("filetypes_video");
}

void TestFileTypes::testFileTypes_video_s29p()
{
    StartTestSuite();
    ConfigOverruleString overruleFrameRate(Config::sPathDefaultFrameRate, FrameRate::s30p.toString());
    executeTest("filetypes_video");
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void TestFileTypes::executeTest(wxString filetypesDir)
{
    // Wav files from: http://www-mmsp.ece.mcgill.ca/documents/AudioFormats/WAVE/Samples.html
    // Some files from http://samples.mplayerhq.hu/

    // Create the project (must be done after ConfigOverrule* code)
    model::FolderPtr mRoot = createProject();
    ASSERT(mRoot);
    wxString sSequence( "Sequence" );
    model::SequencePtr mSequence = addSequence( sSequence, mRoot );

    Click(wxPoint(2,2)); // Click in the timeline to give it the focus.
    Zoom level(4);

    wxString sFolder1( "Folder1" );
    model::FolderPtr folder1 = addFolder( sFolder1 );

    // Find input files in dir (must be done after creating a project, due to dependencies on project properties for opening/closing files)
    wxFileName TestFilesPath = wxFileName(SOURCE_ROOT,"");
    TestFilesPath.AppendDir("test");
    TestFilesPath.AppendDir(filetypesDir);
    ASSERT(TestFilesPath.IsDir());
    ASSERT(TestFilesPath.DirExists());
    model::IPaths InputFiles = getSupportedFiles(TestFilesPath);

    for ( model::IPathPtr path : InputFiles )
    {
        StartTest(path->getPath().GetFullName());
        model::Files files1 = addFiles( boost::assign::list_of(path->getPath().GetFullPath()), folder1 );

        model::FilePtr file = files1.front();
        if (file->hasVideo())
        {
            DragFromProjectViewToTimeline( file,  getTimeline().GetScreenPosition() - getTimeline().getScrolling().getOffset()  + wxPoint(0, VCenter(VideoTrack(0))) );
        }
        else
        {
            ASSERT(file->hasAudio());
            DragFromProjectViewToTimeline( file,  getTimeline().GetScreenPosition() - getTimeline().getScrolling().getOffset()  + wxPoint(0, VCenter(AudioTrack(0))) );
        }
        if (file->hasVideo())
        {
            ASSERT_VIDEOTRACK0(VideoClip);
            ASSERT_EQUALS(VideoTrack(0)->getLength(),VideoClip(0,0)->getLength());
        }
        else
        {
            ASSERT_ZERO(VideoTrack(0)->getLength());
        }
        if (file->hasAudio())
        {
            ASSERT_AUDIOTRACK0(AudioClip);
            ASSERT_EQUALS(AudioTrack(0)->getLength(),AudioClip(0,0)->getLength());
        }
        else
        {
            ASSERT_ZERO(AudioTrack(0)->getLength());
        }
        if (file->hasVideo())
        {
            Play(HCenter(VideoClip(0,0)), 1000);
        }
        else
        {
            Play(HCenter(AudioClip(0,0)), 1000);
        }
        Undo(2);
    }

    InputFiles.clear();
    mRoot.reset();
    mSequence.reset();
}

} // namespace