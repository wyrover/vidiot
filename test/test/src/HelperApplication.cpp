#include "HelperApplication.h"

#include <wx/cmdproc.h>
#include "Application.h"

namespace test {

void waitForIdle()
{
    static_cast<gui::Application*>(wxTheApp)->waitForIdle();
}

wxString randomString(int length)
{
    srand((unsigned)time(0)); 
    static const wxString alphanum = "0123456789" "ABCDEFGHIJKLMNOPQRSTUVWXYZ" "abcdefghijklmnopqrstuvwxyz";

    wxString result;
    for (int i = 0; i < length; ++i) 
    {
        result += alphanum.GetChar(rand() % (sizeof(alphanum) - 1));
    }
    return result;
}

void pause(int ms)
{
    boost::this_thread::sleep(boost::posix_time::milliseconds(ms));
}

} // namespace