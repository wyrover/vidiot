#include "HelperProjectView.h"

namespace test {

std::pair<RandomTempDirPtr, wxFileName> SaveProjectAndClose()
{
    RandomTempDirPtr tempDirProject = RandomTempDir::generate();
    wxFileName filename = tempDirProject->getFileName();
    filename.SetName("LoadSave");
    filename.SetExt("vid");
    gui::Window::get().GetDocumentManager()->GetCurrentDocument()->SetFilename(filename.GetFullPath());
    gui::Window::get().GetDocumentManager()->GetCurrentDocument()->OnSaveDocument(filename.GetFullPath());
    waitForIdle();
    triggerMenu(wxID_CLOSE);
    waitForIdle();
    return std::make_pair(tempDirProject, filename);
}

} // namespace