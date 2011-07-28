#ifndef HELPER_WINDOW_H
#define HELPER_WINDOW_H

#include <wx/menu.h>
#include <wx/uiaction.h>
#include <boost/shared_ptr.hpp>
#include "UtilLog.h"
#include "Window.h"

namespace model {
class Folder;
typedef boost::shared_ptr<Folder> FolderPtr;
class Sequence;
typedef boost::shared_ptr<Sequence> SequencePtr;
}

namespace gui { namespace timeline {
    class Timeline;
}}

namespace test {

class HelperWindow
    :   public wxUIActionSimulator
{
public:

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

    /// Generate a random string using alphanumeric characters of size length
    static wxString randomString(int length = 8);

    /// Delay the test for 60 seconds to allow using the GUI (debugging)
    static void pause(int ms = 60000);

    /// \return Currently active seqence (the sequence for which the timeline is the active notebook page)
    static model::SequencePtr getSequence();

    /// Return the currently active sequence menu
    static wxMenu* getSequenceMenu();

    /// Return the opened timeline for a sequence
    /// \param sequence if this equals 0 then the active timeline is returned
    static gui::timeline::Timeline& getTimeline(model::SequencePtr sequence = model::SequencePtr());

    /// Trigger an undo via pressing CTRL-z
    static void triggerUndo();

    /// Trigger a redo via pressing CTRL-y
    static void triggerRedo();

    // todo make these methods also base members
    template <class COMMAND>
    void ASSERT_CURRENT_COMMAND_TYPE()
    {
        wxCommand* cmd = gui::Window::get().GetDocumentManager()->GetCurrentDocument()->GetCommandProcessor()->GetCurrentCommand();
        COMMAND* c = dynamic_cast<COMMAND*>(cmd);
        ASSERT(c)(c);
    };

    /// Log the command history
    static void logHistory();
};

} // namespace

#endif // HELPER_WINDOW_H