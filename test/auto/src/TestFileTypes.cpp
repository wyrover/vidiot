#include "TestFileTypes.h"

#include <boost/assign/list_of.hpp>
#include <boost/foreach.hpp>
#include <wx/uiaction.h>
#include "AutoFolder.h"
#include "File.h"
#include "FixtureGui.h"
#include "HelperConfig.h"
#include "HelperApplication.h"
#include "HelperProjectView.h"
#include "HelperTestSuite.h"
#include "HelperTimeline.h"
#include "HelperTimelinesView.h"
#include "HelperTimelineAssert.h"
#include "ProjectViewDropSource.h"
#include "HelperWindow.h"
#include "Layout.h"
#include "Scrolling.h"
#include "UtilLog.h"
#include "AudioTrack.h"
#include "VideoClip.h"
#include "AudioClip.h"
#include "Sequence.h"
#include "Timeline.h"
#include "VideoTrack.h"
#include "ids.h"

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

//RUNONLY(testFileTypes_1_44100);
void TestFileTypes::testFileTypes_1_44100()
{
    StartTestSuite();
    ConfigOverruleLong overruleChannels(Config::sPathDefaultAudioChannels, 1);
    ConfigOverruleLong overruleSampleRate(Config::sPathDefaultAudioSampleRate, 44100);
    executeTest();
}

//RUNONLY(testFileTypes_2_44100);
void TestFileTypes::testFileTypes_2_44100()
{
    StartTestSuite();
    ConfigOverruleLong overruleChannels(Config::sPathDefaultAudioChannels, 2);
    ConfigOverruleLong overruleSampleRate(Config::sPathDefaultAudioSampleRate, 44100);
    executeTest();
}

//RUNONLY(testFileTypes_1_48000);
void TestFileTypes::testFileTypes_1_48000()
{
    StartTestSuite();
    ConfigOverruleLong overruleChannels(Config::sPathDefaultAudioChannels, 1);
    ConfigOverruleLong overruleSampleRate(Config::sPathDefaultAudioSampleRate, 48000);
    executeTest();
}

//RUNONLY(testFileTypes_2_48000);
void TestFileTypes::testFileTypes_2_48000()
{
    StartTestSuite();
    ConfigOverruleLong overruleChannels(Config::sPathDefaultAudioChannels, 2);
    ConfigOverruleLong overruleSampleRate(Config::sPathDefaultAudioSampleRate, 48000);
    executeTest();
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void TestFileTypes::executeTest()
{
    // Create the project (must be done after ConfigOverrule* code)
    model::FolderPtr mRoot = createProject();
    ASSERT(mRoot);
    wxString sSequence( "Sequence" );
    model::SequencePtr mSequence = addSequence( sSequence, mRoot );
    wxString sFolder1( "Folder1" );
    model::FolderPtr folder1 = addFolder( sFolder1 );

    // Find input files in dir (must be done after creating a project, due to dependencies on project properties for opening/closing files)
    wxFileName TestFilesPath = wxFileName(SOURCE_ROOT,"");
    TestFilesPath.AppendDir("test");
    TestFilesPath.AppendDir("filetypes");
    ASSERT(TestFilesPath.IsDir());
    ASSERT(TestFilesPath.DirExists());
    model::IPaths InputFiles = getSupportedFiles(TestFilesPath);

    BOOST_FOREACH( model::IPathPtr path, InputFiles )
    {
        StartTest(path->getPath().GetFullName());
        model::Files files1 = addFiles( boost::assign::list_of(path->getPath().GetFullPath()), folder1 );

        DragFromProjectViewToTimeline( files1.front(),  getTimeline().GetScreenPosition() - getTimeline().getScrolling().getOffset()  + wxPoint(3, VCenter(VideoTrack(0))) );

        ASSERT_VIDEOTRACK0(VideoClip);
        ASSERT_EQUALS(VideoTrack(0)->getLength(),VideoClip(0,0)->getLength());
        ASSERT_AUDIOTRACK0(AudioClip);
        ASSERT_EQUALS(AudioTrack(0)->getLength(),AudioClip(0,0)->getLength());
        Play(HCenter(VideoClip(0,0)), 1000);
        Undo(2);
    }

    InputFiles.clear();
    mRoot.reset();
    mSequence.reset();
}

} // namespace