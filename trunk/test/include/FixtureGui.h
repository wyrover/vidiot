#ifndef FIXTURE_GUI_H
#define FIXTURE_GUI_H

#include <cxxtest/GlobalFixture.h>
#include <wx/filename.h>
#include <wx/window.h>
#include <boost/thread.hpp>
#include <boost/thread/barrier.hpp>
#include "IEventLoopListener.h"

namespace model {
class AProjectViewNode;
typedef boost::shared_ptr<AProjectViewNode> ProjectViewPtr;
class Folder;
typedef boost::shared_ptr<Folder> FolderPtr;
class Sequence;
typedef boost::shared_ptr<Sequence> SequencePtr;
class File;
typedef boost::shared_ptr<File> FilePtr;
typedef std::list<FilePtr> Files;
}

namespace test {

/// Fixture for the complete GUI. The complete application is started, with the
/// main thread running in a separate thread so that tests can be ran from the main
/// application thread. This fixture also ensures that test execution waits 
/// until that event loop is running properly (the wxWidgets part). The implementation
/// also ensures that a new window is used for each new test.
class FixtureGui 
    :   public CxxTest::GlobalFixture
    ,   public IEventLoopListener
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    static FixtureGui sInstance;

    FixtureGui();
    virtual ~FixtureGui();

    virtual bool setUpWorld();      ///< Called before first test. Starts Application() and the main wxWidgets thread
    virtual bool tearDownWorld();   ///< Called after last test. Closes main wxWidgets thread and Application()

    virtual bool setUp();       ///< Called before each test. Starts the window.
    virtual bool tearDown();    ///< Called after each test. Closes the window.

    //////////////////////////////////////////////////////////////////////////
    // IEventLoopListener
    //////////////////////////////////////////////////////////////////////////

    void onEventLoopEnter();    ///< Receive notification that the OnRun thread is running the wxWidgets event loop for the (re)started Window

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    /// Trigger the menu given. That menu should be in the main menu bar.
    /// \param id menu identifier
    static void triggerMenu(int id);

    /// Trigger the menu given on the window given.
    /// \param id menu identifier
    static void triggerMenu(wxWindow& window, int id);

    /// Wait until the main application loop signals Idle via a idle event.
    /// If the loop is already idle, an extra idle event is triggered.
    static void waitForIdle();

    /// Create a new project in a blank application by triggering File->New
    /// \return root node of the project
    static model::FolderPtr createProject();

    /// \return root node of the project
    static model::FolderPtr getRoot();

    /// Create a new autofolder to the given path in a given parent folder or in the root (default)
    /// \return new autofolder
    static model::FolderPtr addAutoFolder( wxFileName path, model::FolderPtr parent = getRoot() );

    /// Create a new named folder in a given parent folder or in the root (default)
    /// \return new folder
    static model::FolderPtr addFolder( wxString name, model::FolderPtr parent = getRoot() );

    /// Create a new sequence in a given parent folder or in the root (default)
    /// \return new sequence
    static model::SequencePtr addSequence( wxString name, model::FolderPtr parent = getRoot() );

    /// Create a new sequence from a given folder
    /// \return new sequence
    static model::SequencePtr createSequence( model::FolderPtr folder );

    /// Create new files in a given parent folder or in the root (default)
    /// \return new files created in the model
    static model::Files addFiles( std::list<wxFileName> name, model::FolderPtr parent = getRoot() );

    /// Remove given node from the project view via selecting it and then triggering the delete menu option
    /// \node node to be removed
    static void remove( model::ProjectViewPtr node );

    /// Count the number of nodes currently visible in the project view.
    /// This is implemented by selecting all nodes, and counting the selection size.
    /// Thus, as a side effect changes the selection of the project view.
    static int countProjectView();

    /// Generate a random string using alphanumeric characters of size length
    static wxString randomString(int length = 8);

    /// Delay the test for 60 seconds to allow using the GUI (debugging)
    static void pause();

private:

    //////////////////////////////////////////////////////////////////////////
    // MAIN WXWIDGETS THREAD
    //////////////////////////////////////////////////////////////////////////

    void mainThread();

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    boost::scoped_ptr<boost::thread> mThread;

    boost::barrier mBarrierStart;
    boost::barrier mBarrierStarted;
    boost::barrier mBarrierStopped;

    bool mEnd;
};

} // namespace

#endif // FIXTURE_GUI_H