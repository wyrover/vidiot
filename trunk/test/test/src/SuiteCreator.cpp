#include "SuiteCreator.h"
#include "Window.h"

namespace test {

void setTitle(const char* title)
{
    static int testcount = 1;
    wxString s;
    s << testcount << " - " << title;
    gui::Window::get().setAdditionalTitle(s);
    testcount++;
}

} // namespace