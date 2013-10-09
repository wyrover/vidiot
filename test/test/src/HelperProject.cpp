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

#include "HelperProjectView.h"

#include "Application.h"
#include "UtilThread.h"

namespace test {

std::pair<RandomTempDirPtr, wxFileName> SaveProjectAndClose()
{
    RandomTempDirPtr tempDirProject = RandomTempDir::generate();
    wxFileName filename = tempDirProject->getFileName();
    filename.SetName(wxString::Format(wxT("%s_%d"), gui::Application::getVersion(), gui::Application::getRevision()));
    filename.SetExt("vid");
    util::thread::RunInMainAndWait([filename]()
    {
        gui::Window::get().GetDocumentManager()->GetCurrentDocument()->SetFilename(filename.GetFullPath());
        gui::Window::get().GetDocumentManager()->GetCurrentDocument()->OnSaveDocument(filename.GetFullPath());
    });
    waitForIdle();
    triggerMenu(wxID_CLOSE);
    waitForIdle();
    return std::make_pair(tempDirProject, filename);
}

void CloseDocumentAndAvoidSaveDialog()
{
    wxDocument* doc = gui::Window::get().GetDocumentManager()->GetCurrentDocument();
    if (doc)
    {
        doc->Modify(false); // Avoid "Save yes/no/Cancel" dialog
    }
    triggerMenu(wxID_CLOSE);
}

} // namespace