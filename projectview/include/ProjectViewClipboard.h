// Copyright 2014 Eric Raijmakers.
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

#ifndef PROJECT_VIEW_CLIPBOARD_H
#define PROJECT_VIEW_CLIPBOARD_H

namespace gui {

class ProjectView;

class ProjectViewClipboard
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    ProjectViewClipboard(ProjectView& parent);
    virtual ~ProjectViewClipboard();

    //////////////////////////////////////////////////////////////////////////
    // MAIN WINDOW EDIT MENU
    //////////////////////////////////////////////////////////////////////////

    void onCutFromMainMenu(wxCommandEvent& event);
    void onCopyFromMainMenu(wxCommandEvent& event);
    void onPasteFromMainMenu(wxCommandEvent& event);

    //////////////////////////////////////////////////////////////////////////
    // POPUP MENU
    //////////////////////////////////////////////////////////////////////////

    void onCut();
    void onCopy();
    void onPaste();

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

	ProjectView& mProjectView;

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    bool hasKeyboardFocus() const;

    /// Check if the current selection can be stored in the clipboard and
    /// return true if one or more nodes were stored.
    /// \return true if the selection was stored in the clipboard
    bool storeSelectionInClipboard() const;

    void pasteFromClipboard();
};

} // namespace

#endif
