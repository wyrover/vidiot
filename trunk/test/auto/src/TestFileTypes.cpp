#include "TestFileTypes.h"

#include <boost/assign/list_of.hpp>
#include <boost/foreach.hpp>
#include <wx/uiaction.h>
#include "AutoFolder.h"
#include "File.h"
#include "FixtureGui.h"
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
    if (!HelperTestSuite::get().currentTestIsEnabled()) { return; } // Test was disabled

#ifndef SOURCE_ROOT
#error "SOURCE_ROOT is not defined!"
#endif

    TestFilesPath = wxFileName(SOURCE_ROOT,"");
    TestFilesPath.AppendDir("test");
    TestFilesPath.AppendDir("filetypes");
    ASSERT(TestFilesPath.IsDir());
    ASSERT(TestFilesPath.DirExists());

    mRoot = createProject();
    ASSERT(mRoot);

    wxString sSequence( "Sequence" );
    mSequence = addSequence( sSequence, mRoot );
    triggerMenu(ID_ADDVIDEOTRACK);
    triggerMenu(ID_ADDAUDIOTRACK);

    InputFiles = model::AutoFolder::getSupportedFiles(TestFilesPath);
}

void TestFileTypes::tearDown()
{
    InputFiles.clear();
    mRoot.reset();
    mSequence.reset();
}

//////////////////////////////////////////////////////////////////////////
// TEST CASES
//////////////////////////////////////////////////////////////////////////

//RUNONLY(testFileTypes);
void TestFileTypes::testFileTypes()
{
    StartTestSuite();

    wxString sFolder1( "Folder1" );
    model::FolderPtr folder1 = addFolder( sFolder1 );

    BOOST_FOREACH( model::IPathPtr path, InputFiles )
    {
        StartTest(path->getPath().GetFullName());
        model::Files files1 = addFiles( boost::assign::list_of(path->getPath().GetFullPath()), folder1 );

        DragFromProjectViewToTimeline( files1.front(),  getTimeline().GetScreenPosition() - getTimeline().getScrolling().getOffset()  + wxPoint(3, VCenter(VideoTrack(0))) );

        ASSERT_VIDEOTRACK0(VideoClip);
        ASSERT_EQUALS(VideoTrack(0)->getLength(),VideoClip(0,0)->getLength());
        ASSERT_AUDIOTRACK0(AudioClip);
        ASSERT_EQUALS(AudioTrack(0)->getLength(),AudioClip(0,0)->getLength());
        Play(HCenter(VideoClip(0,0)), 2000);
        Undo(2);
    }
}

// todo make test for '2 44100 stereo' '2 48000 stereo' '2 44100 mono' and '2 48000 mono'
} // namespace