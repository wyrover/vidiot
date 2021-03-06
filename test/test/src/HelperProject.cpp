// Copyright 2013-2016 Eric Raijmakers.
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

void SetProjectUnmodified()
{
    util::thread::RunInMainAndWait([]
    {
        wxDocument* doc = gui::Window::get().GetDocumentManager()->GetCurrentDocument();
        if (doc) { doc->Modify(false); } // Avoid "Save yes/no/Cancel" dialog

        // Clear history also since there are often crashes on saving the history
        // when running the tests under GTK.
        wxFileHistory* history = gui::Window::get().GetDocumentManager()->GetFileHistory();
        while (history->GetCount() > 0)
        {
            history->RemoveFileFromHistory(0);
        }
    });
}

model::FolderPtr getRoot()
{
    model::FolderPtr root = model::Project::get().getRoot();
    return boost::static_pointer_cast<model::Folder>(root);
}

void OpenProject(wxString path)
{
    util::thread::RunInMainAndWait([path]()
    {
        gui::Window::get().GetDocumentManager()->CreateDocument(path, wxDOC_SILENT);
    });
    WaitForIdle;
}

model::SequencePtr CreateProjectWithClosedSequence()
{
    model::FolderPtr root = WindowCreateProject();
    ASSERT(root);
    wxString sSequence("Sequence");
    model::SequencePtr sequence{ ProjectViewAddSequence(sSequence, root) };
    WindowTriggerMenu(ID_CLOSESEQUENCE);
    return sequence;
}

DirAndFile SaveProjectAndClose(boost::optional<RandomTempDirPtr> tempDir, wxString filesuffix)
{
    DirAndFile tempDir_fileName = SaveProject(tempDir,filesuffix);
    WindowTriggerMenu(wxID_CLOSE);
    WaitForIdle;
    ASSERT(!model::Project::exists());
    ASSERT(!model::Root::exists());
    return tempDir_fileName;
}

DirAndFile SaveProject(boost::optional<RandomTempDirPtr> tempDir, wxString filesuffix)
{
    RandomTempDirPtr tempDirProject = tempDir ? *tempDir : RandomTempDir::generate();
    wxFileName filename = generateSaveFileName(tempDirProject->getFileName());
    filename.SetName(filename.GetName() + filesuffix);
    util::thread::RunInMainAndWait([filename]()
    {
        // Causes occasional crashes (can't write to recent file list):
        // gui::Window::get().GetDocumentManager()->GetCurrentDocument()->SetFilename(filename.GetLongPath());
        gui::Window::get().GetDocumentManager()->GetCurrentDocument()->OnSaveDocument(filename.GetLongPath());
    });
    return std::make_pair(tempDirProject, filename);
}

wxFileName generateSaveFileName(wxFileName dir)
{
    ASSERT(dir.IsDir());
    wxFileName result(dir);
    result.SetName(wxString::Format(wxT("%s_%d"), gui::Application::getVersion(), gui::Application::getRevision()));
    result.SetExt(model::Project::sFileExtension);
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
    WaitForIdle;
}

} // namespace
