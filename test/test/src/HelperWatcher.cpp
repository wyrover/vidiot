#include "HelperWatcher.h"

#include "HelperThread.h"
#include "UtilLog.h"
#include "Watcher.h"

namespace test {

void ASSERT_WATCHED_PATHS_COUNT(int n)
{
    RunInMainThread([n] { ASSERT_EQUALS(gui::Watcher::get().getWatchedPathsCount(),n); });
}

} // namespace