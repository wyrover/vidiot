// Copyright 2013,2014 Eric Raijmakers.
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

#include "Test.h"

namespace test {

model::FolderPtr getRoot()
{
    model::FolderPtr root = model::Project::get().getRoot();
    return boost::static_pointer_cast<model::Folder>(root);
}

DirAndFile SaveProjectAndClose(boost::optional<RandomTempDirPtr> tempDir)
{
    DirAndFile tempDir_fileName = SaveProject(tempDir);
    WindowTriggerMenu(wxID_CLOSE);
    WaitForIdle();
    return tempDir_fileName;
}

DirAndFile SaveProject(boost::optional<RandomTempDirPtr> tempDir)
{
    RandomTempDirPtr tempDirProject = tempDir ? *tempDir : RandomTempDir::generate();
    wxFileName filename = generateSaveFileName(tempDirProject->getFileName());
    RunInMainAndWait([filename]()
    {
        gui::Window::get().GetDocumentManager()->GetCurrentDocument()->SetFilename(filename.GetFullPath());
        gui::Window::get().GetDocumentManager()->GetCurrentDocument()->OnSaveDocument(filename.GetFullPath());
    });
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

void CloseProjectAndAvoidSaveDialog()
{
    wxDocument* doc = gui::Window::get().GetDocumentManager()->GetCurrentDocument();
    if (doc)
    {
        doc->Modify(false); // Avoid "Save yes/no/Cancel" dialog
    }
    WindowTriggerMenu(wxID_CLOSE);
}

} // namespace