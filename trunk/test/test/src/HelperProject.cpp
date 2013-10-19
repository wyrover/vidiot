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

#include "HelperProject.h"

#include "Application.h"
#include "UtilThread.h"

namespace test {

DirAndFile SaveProjectAndClose(boost::optional<RandomTempDirPtr> tempDir)
{
    DirAndFile tempDir_fileName = SaveProject(tempDir);
    triggerMenu(wxID_CLOSE);
    waitForIdle();
    return tempDir_fileName;
}

DirAndFile SaveProject(boost::optional<RandomTempDirPtr> tempDir)
{
    RandomTempDirPtr tempDirProject = tempDir ? *tempDir : RandomTempDir::generate();
    wxFileName filename = generateSaveFileName(tempDirProject->getFileName());
    util::thread::RunInMainAndWait([filename]()
    {
        gui::Window::get().GetDocumentManager()->GetCurrentDocument()->SetFilename(filename.GetFullPath());
        gui::Window::get().GetDocumentManager()->GetCurrentDocument()->OnSaveDocument(filename.GetFullPath());
    });
    waitForIdle();
    return std::make_pair(tempDirProject, filename);
}

wxFileName generateSaveFileName(wxFileName dir)
{
    ASSERT(dir.IsDir());
    wxFileName result(dir);
    result.SetName(wxString::Format(wxT("%s_%d"), gui::Application::getVersion(), gui::Application::getRevision()));
    result.SetExt("vid");
    return result;
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