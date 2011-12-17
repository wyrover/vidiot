#include "SuiteCreator.h"
#include "Window.h"

namespace test {

void setTitle(const char* title)
{
    gui::Window::get().setAdditionalTitle(wxString(title));
}

} // namespace