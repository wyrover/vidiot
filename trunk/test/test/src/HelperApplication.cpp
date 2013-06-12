#include "HelperApplication.h"

#include "Application.h"
#include "UtilLog.h"

namespace test {

void waitForIdle()
{
    static_cast<gui::Application*>(wxTheApp)->waitForIdle();
}

void pause(int ms)
{
    VAR_WARNING(ms);
    boost::this_thread::sleep(boost::posix_time::milliseconds(ms));
}

} // namespace