#ifndef HELPER_WINDOW_H
#define HELPER_WINDOW_H

#include <wx/cmdproc.h> 
#include <wx/frame.h>
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

/// Check the menu given. That menu should be in the main menu bar.
/// Note that the corresponding event will always be triggered, even
/// if the control indicated with 'id' already has the value indicated
/// by 'checked'.
/// \param id menu identifier
/// \param checked if true, the menu is checked, unchecked otherwise
void checkMenu(int id, bool checked = true);

/// Check the menu given on the window given.
/// Note that the corresponding event will always be triggered, even
/// if the control indicated with 'id' already has the value indicated
/// by 'checked'.
/// \param id menu identifier
/// \param checked if true, the menu is checked, unchecked otherwise
void checkMenu(wxFrame& window, int id, bool checked = true);

/// Create a new project in a blank application by triggering File->New
/// \return root node of the project
model::FolderPtr createProject();

/// \return root node of the project
model::FolderPtr getRoot();

/// Return the currently active sequence menu
wxMenu* getSequenceMenu();

/// Trigger an undo via pressing CTRL-z
void Undo();

/// Trigger a redo via pressing CTRL-y
void Redo();

/// Return current command in the history
wxCommand* getCurrentCommand();

/// Assert if the current active command in the history
/// (the command that will be undone if undo is triggered)
/// is of the given type.
template <class COMMAND>
void ASSERT_CURRENT_COMMAND_TYPE()
{
    COMMAND* command = dynamic_cast<COMMAND*>(getCurrentCommand());
    ASSERT(command);
};

/// Log the command history
void logHistory();

void ControlDown();     ///< Press down CTRL
void ControlUp();       ///< Release CTRL

void ShiftDown();       ///< Press down Shift
void ShiftUp();         ///< Release Shift

void Type(int keycode, int modifiers = wxMOD_NONE); 	

} // namespace

#endif // HELPER_WINDOW_H