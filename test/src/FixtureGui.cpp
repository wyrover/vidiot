#include "FixtureGui.h"

#include <cstdlib>
#include <ctime> 
#include <wx/evtloop.h> 
#include <boost/assign/list_of.hpp>
#include <boost/foreach.hpp>
#include "Application.h"
#include "AutoFolder.h"
#include "ClipView.h"
#include "EmptyClip.h"
#include "File.h"
#include "IClip.h"
#include "ids.h"
#include "Project.h"
#include "ProjectView.h"
#include "Selection.h"
#include "Sequence.h"
#include "SequenceView.h"
#include "Timeline.h"
#include "TimeLinesView.h"
#include "Track.h"
#include "UtilDialog.h"
#include "UtilLog.h"
#include "ViewMap.h"
#include "Window.h"

namespace test {

//static 
FixtureGui sInstance;

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

FixtureGui::FixtureGui()
    :   mEnd(false)
    ,   mBarrierStart(2)
    ,   mBarrierStarted(2)
    ,   mBarrierStopped(2)
{
}

FixtureGui::~FixtureGui()
{
}

bool FixtureGui::setUpWorld()
{
    mThread.reset(new boost::thread(boost::bind(&FixtureGui::mainThread,this)));
    return true;
}

bool FixtureGui::tearDownWorld()
{
    ASSERT(!mEnd);
    mEnd = true;
    mBarrierStart.wait();
    if (mThread)
    {
        mThread->join();
    }
    return true;
}

bool FixtureGui::setUp()
{
    mBarrierStart.wait();
    mBarrierStarted.wait();
    return true;
}

bool FixtureGui::tearDown()
{
    wxDocument* doc = gui::Window::get().GetDocumentManager()->GetCurrentDocument();
    if (doc)
    {
        doc->Modify(false); // Avoid "Save yes/no/Cancel" dialog
    }
    // Must be done via an Event, since all wxWindows derived classes must be 
    // destroyed in the same thread as in which they were created, which is 
    // the main wxWidgets event thread.
    FixtureGui::triggerMenu(wxID_EXIT);

    // The exit causes the event loop to be activated again, resulting in one
    // extra call to onEventLoopEnter. That must be unblocked.
    mBarrierStarted.wait();

    // Wait until main thread 'OnRun' stopped
    mBarrierStopped.wait();
    return true;
}

//////////////////////////////////////////////////////////////////////////
// IEventLoopListener
//////////////////////////////////////////////////////////////////////////

void FixtureGui::onEventLoopEnter()
{
    mBarrierStarted.wait();
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

// static
void FixtureGui::triggerMenu(int id)
{
    triggerMenu(gui::Window::get(), id);
}

// static
void FixtureGui::triggerMenu(wxWindow& window, int id)
{
    window.GetEventHandler()->QueueEvent(new wxCommandEvent(wxEVT_COMMAND_MENU_SELECTED,id));
    waitForIdle();
}

// static
void FixtureGui::waitForIdle()
{
    static_cast<gui::Application*>(wxTheApp)->waitForIdle();
}

// static
model::FolderPtr FixtureGui::createProject()
{
    waitForIdle();
    triggerMenu(wxID_NEW);
    waitForIdle();
    return getRoot();
}

// static
model::FolderPtr FixtureGui::getRoot()
{
    model::FolderPtr root = model::Project::get().getRoot();
    return boost::static_pointer_cast<model::Folder>(root);
}

// static
model::FolderPtr FixtureGui::addAutoFolder( wxFileName path, model::FolderPtr parent )
{
    waitForIdle();
    gui::ProjectView::get().select(boost::assign::list_of(parent));
    waitForIdle();
    UtilDialog::setDir( path.GetShortPath() ); // Add with short path
    triggerMenu(gui::ProjectView::get(),meID_NEW_AUTOFOLDER);
    waitForIdle();

    model::NodePtrs nodes = getRoot()->find( path.GetLongPath() ); // Converted to long path in vidiot
    ASSERT(nodes.size() == 1);
    model::NodePtr node = nodes.front();
    ASSERT(node->isA<model::AutoFolder>())(node);
    model::FolderPtr folder = boost::static_pointer_cast<model::Folder>(node);
    return folder;
}

// static
model::FolderPtr FixtureGui::addFolder( wxString name, model::FolderPtr parent )
{
    waitForIdle();
    gui::ProjectView::get().select(boost::assign::list_of(parent));
    UtilDialog::setText( name );
    triggerMenu(gui::ProjectView::get(),meID_NEW_FOLDER);
    waitForIdle();

    model::NodePtrs nodes = getRoot()->find( name );
    ASSERT(nodes.size() == 1);
    model::NodePtr node = nodes.front();
    ASSERT(node->isA<model::Folder>())(node);
    model::FolderPtr folder = boost::static_pointer_cast<model::Folder>(node);
    return folder;
}

// static
model::SequencePtr FixtureGui::addSequence( wxString name, model::FolderPtr parent )
{
    waitForIdle();
    gui::ProjectView::get().select(boost::assign::list_of(parent));
    UtilDialog::setText( name );
    triggerMenu(gui::ProjectView::get(),meID_NEW_SEQUENCE);
    waitForIdle();

    model::NodePtrs nodes = getRoot()->find( name );
    ASSERT(nodes.size() == 1);
    model::NodePtr node = nodes.front();
    ASSERT(node->isA<model::Sequence>())(node);
    model::SequencePtr sequence = boost::static_pointer_cast<model::Sequence>(node);
    return sequence;
}

// static 
model::SequencePtr FixtureGui::createSequence( model::FolderPtr folder )
{
    waitForIdle();
    gui::ProjectView::get().select(boost::assign::list_of(folder));
    waitForIdle();
    triggerMenu(gui::ProjectView::get(),meID_CREATE_SEQUENCE);
    waitForIdle();

    model::NodePtrs nodes = getRoot()->find( folder->getName() );
    ASSERT(nodes.size() == 2); // The sequence and the folder
    model::SequencePtr result;
    BOOST_FOREACH( model::NodePtr node, nodes )
    {
        if (node->isA<model::Sequence>())
        {
            result = boost::static_pointer_cast<model::Sequence>(node);
        }
    }
    ASSERT(result);
    return result;
}

// static
model::Files FixtureGui::addFiles( std::list<wxFileName> paths, model::FolderPtr parent )
{
    waitForIdle();
    gui::ProjectView::get().select(boost::assign::list_of(parent));
    std::list<wxString> shortpaths;
    BOOST_FOREACH( wxFileName path, paths )
    {
        ASSERT( path.IsAbsolute() );
        shortpaths.push_back( path.GetShortPath() ); // Add with short path
    }
    UtilDialog::setFiles( shortpaths );
    triggerMenu(gui::ProjectView::get(),meID_NEW_FILE);
    waitForIdle();

    model::Files result;
    BOOST_FOREACH( wxFileName path, paths )
    {
        model::NodePtrs nodes = parent->find( path.GetLongPath() ); // Check that long path is generated by vidiot
        ASSERT(nodes.size() == 1);
        model::NodePtr node = nodes.front();
        ASSERT(node->isA<model::File>())(node);
        result.push_back(boost::static_pointer_cast<model::File>(node));
    }
    return result;
}

// static
void FixtureGui::remove( model::NodePtr node )
{
    waitForIdle();
    gui::ProjectView::get().select(boost::assign::list_of(node));
    waitForIdle();
    triggerMenu(gui::ProjectView::get(),wxID_DELETE);
    waitForIdle();
}


// static 
int FixtureGui::countProjectView()
{
    FixtureGui::waitForIdle();
    gui::ProjectView::get().selectAll();
    model::NodePtrs selection = gui::ProjectView::get().getSelection();
    int result = selection.size();
    VAR_DEBUG(result);
    return result;
}

// static
wxString FixtureGui::randomString(int length)
{
    srand((unsigned)time(0)); 
    static const wxString alphanum = "0123456789" "ABCDEFGHIJKLMNOPQRSTUVWXYZ" "abcdefghijklmnopqrstuvwxyz";

    wxString result;
    for (int i = 0; i < length; ++i) 
    {
        result += alphanum.GetChar(rand() % (sizeof(alphanum) - 1));
    }
    return result;
}

// static 
void FixtureGui::pause(int ms)
{
    boost::this_thread::sleep(boost::posix_time::milliseconds(ms));
}

// static 
model::SequencePtr FixtureGui::getActiveSequence()
{
    return getTimeline().getSequence();
}

// static
wxMenu* FixtureGui::getSequenceMenu()
{
    return gui::Window::get().GetMenuBar()->GetMenu(gui::Window::sSequenceMenuIndex);
}

// static
gui::timeline::Timeline& FixtureGui::getTimeline(model::SequencePtr sequence)
{
    if (!sequence)
    {
        return *gui::TimelinesView::get().getTimeline(0);
    }
    return *gui::TimelinesView::get().findPage(sequence).second;
}

// static 
int FixtureGui::getNumberOfClipsInVideoTrack(int trackindex)
{
    model::TrackPtr videoTrack = getActiveSequence()->getVideoTrack(trackindex);
    return videoTrack->getClips().size();
}

// static 
model::IClipPtr FixtureGui::getVideoClip(int trackindex, int clipindex)
{
    model::TrackPtr videoTrack = getActiveSequence()->getVideoTrack(trackindex);
    return videoTrack->getClipByIndex(clipindex);
}

//static 
int FixtureGui::getNonEmptyClipsCount(model::SequencePtr sequence)
{
    int result = 0;
    BOOST_FOREACH( model::TrackPtr track, getTimeline(sequence).getSequence()->getTracks() )
    {
        BOOST_FOREACH( model::IClipPtr clip, track->getClips() )
        {
            if (!clip->isA<model::EmptyClip>())
            {
                result++;
            }
        }
    }
    return result;
}

//static 
int FixtureGui::getSelectedClipsCount(model::SequencePtr sequence)
{
    int result = getTimeline(sequence).getSelection().getClips().size();
    VAR_DEBUG(result);
    return result;
}

// static 
pixel FixtureGui::getLeft(model::IClipPtr clip, model::SequencePtr sequence)
{
    return getTimeline(sequence).getViewMap().getView(clip)->getLeftPosition();
}

// static 
pixel FixtureGui::getRight(model::IClipPtr clip, model::SequencePtr sequence)
{
    return getTimeline(sequence).getViewMap().getView(clip)->getRightPosition();
}

// static 
pixel FixtureGui::getTop(model::IClipPtr clip, model::SequencePtr sequence)
{
    return getTimeline(sequence).getSequenceView().getPosition(clip->getTrack());
}

// static 
pixel FixtureGui::getBottom(model::IClipPtr clip, model::SequencePtr sequence)
{
    return getTop(clip,sequence) + clip->getTrack()->getHeight();
}

//////////////////////////////////////////////////////////////////////////
// MAIN WXWIDGETS THREAD
//////////////////////////////////////////////////////////////////////////

void FixtureGui::mainThread()
{
    wxApp::SetInstance(new gui::Application(this));
    int argc = 1;
    char* argv = _strdup(gui::Application::sTestApplicationName);
    wxEntryStart(argc, &argv);

    while (!mEnd)
    {
        mBarrierStart.wait();
        if (!mEnd)
        {
            wxTheApp->CallOnInit();
            wxTheApp->OnRun();
            mBarrierStopped.wait();
        }
    }

    wxEntryCleanup();
}

} // namespace