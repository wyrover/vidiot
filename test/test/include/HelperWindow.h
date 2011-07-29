#ifndef HELPER_WINDOW_H
#define HELPER_WINDOW_H

#include <wx/cmdproc.h> 
#include <wx/menu.h>
#include <wx/window.h>
#include <boost/shared_ptr.hpp>
#include "UtilLog.h"

namespace model {
class Folder;
typedef boost::shared_ptr<Folder> FolderPtr;
}

namespace test {

/// Trigger the menu given. That menu should be in the main menu bar.
/// \param id menu identifier
void triggerMenu(int id);

/// Trigger the menu given on the window given.
/// \param id menu identifier
void triggerMenu(wxWindow& window, int id);

/// Create a new project in a blank application by triggering File->New
/// \return root node of the project
model::FolderPtr createProject();

/// \return root node of the project
model::FolderPtr getRoot();

/// Return the currently active sequence menu
wxMenu* getSequenceMenu();

/// Trigger an undo via pressing CTRL-z
void triggerUndo();

/// Trigger a redo via pressing CTRL-y
void triggerRedo();

/// Return current command in the history
wxCommand* getCurrentCommand();

// todo make these methods also base members
template <class COMMAND>
void ASSERT_CURRENT_COMMAND_TYPE()
{
    COMMAND* command = dynamic_cast<COMMAND*>(getCurrentCommand());
    ASSERT(command);
};

/// Log the command history
void logHistory();

} // namespace

#endif // HELPER_WINDOW_H