#include "UtilThread.h"

namespace util { namespace thread {

void RunInMain(boost::function<void()> method)
{
    RunInMainThread<int>([method] () -> int { method(); return 0; });
}

}} // namespace