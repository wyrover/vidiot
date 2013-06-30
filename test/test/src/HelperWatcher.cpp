#include "HelperWatcher.h"

#include "UtilLog.h"
#include "UtilThread.h"
#include "Watcher.h"

namespace test {

void ASSERT_WATCHED_PATHS_COUNT(int n)
{
    util::thread::RunInMainAndWait([n] 
    { 
        ASSERT_EQUALS(gui::Watcher::get().getWatchedPathsCount(),n); 
    });
}

} // namespace