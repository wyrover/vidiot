#ifndef FIXTURE_GUI_H
#define FIXTURE_GUI_H

#include <cxxtest/GlobalFixture.h>
#include <wx/filename.h>
#include <wx/window.h>
#include <boost/thread.hpp>
#include <boost/thread/barrier.hpp>
#include "IEventLoopListener.h"
#include "UtilInt.h"

namespace model {
class INode;
typedef boost::shared_ptr<INode> NodePtr;
class IClip;
typedef boost::shared_ptr<IClip> IClipPtr;
typedef std::list<IClipPtr> IClips;
class Folder;
typedef boost::shared_ptr<Folder> FolderPtr;
class Sequence;
typedef boost::shared_ptr<Sequence> SequencePtr;
class File;
typedef boost::shared_ptr<File> FilePtr;
typedef std::list<FilePtr> Files;
}

namespace gui { namespace timeline {
    class Timeline;
}}

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
    static void remove( model::NodePtr node );

    /// Count the number of nodes currently visible in the project view.
    /// This is implemented by selecting all nodes, and counting the selection size.
    /// Thus, as a side effect changes the selection of the project view.
    static int countProjectView();

    /// Generate a random string using alphanumeric characters of size length
    static wxString randomString(int length = 8);

    /// Delay the test for 60 seconds to allow using the GUI (debugging)
    static void pause(int ms = 60000);

    /// \return Currently active seqence (the sequence for which the timeline is the active notebook page)
    static model::SequencePtr getActiveSequence();

    /// Return the currently active sequence menu
    static wxMenu* getSequenceMenu();

    /// Return the opened timeline for a sequence
    /// \param sequence if this equals 0 then the active timeline is returned
    static gui::timeline::Timeline& getTimeline(model::SequencePtr sequence = model::SequencePtr());

    /// Return the number of clips in a given video track
    /// \param trackindex index position (0-based) of the video track, counting from the divider upwards
    static int getNumberOfClipsInVideoTrack(int trackindex = 0);

    /// Return a clip in a video track
    /// \param trackindex index position (0-based) of the video track, counting from the divider upwards
    /// \param clipindex index position (0-based) of the clip in the track, counting from left to right
    static model::IClipPtr getVideoClip(int trackindex = 0, int clipindex = 0);
    
    /// Count non-empty clips (both audio and video) in given timeline
    /// \param sequence if 0 then the active timeline is used
    static int getNonEmptyClipsCount(model::SequencePtr sequence  = model::SequencePtr());

    /// Count selected clips (both audio and video) in given timeline
    /// \param sequence if 0 then the active timeline is used
    static int getSelectedClipsCount(model::SequencePtr sequence  = model::SequencePtr());

    /// Return left x position of given clip
    /// \param sequence if 0 then the active timeline is used
    /// \param clip given clip
    static pixel getLeft(model::IClipPtr clip, model::SequencePtr sequence  = model::SequencePtr());

    /// Return right x position of given clip
    /// \param sequence if 0 then the active timeline is used
    /// \param clip given clip
    static pixel getRight(model::IClipPtr clip, model::SequencePtr sequence  = model::SequencePtr());

    /// Return top y position of given clip
    /// \param sequence if 0 then the active timeline is used
    /// \param clip given clip
    static pixel getTop(model::IClipPtr clip, model::SequencePtr sequence  = model::SequencePtr());

    /// Return bottom y position of given clip
    /// \param sequence if 0 then the active timeline is used
    /// \param clip given clip
    static pixel getBottom(model::IClipPtr clip, model::SequencePtr sequence  = model::SequencePtr());

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