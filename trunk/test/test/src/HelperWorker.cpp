#include "HelperWorker.h"

#include "Worker.h"

namespace test {

ExpectExecutedWork::ExpectExecutedWork(int nWork)
    : mWork(nWork)
    , mWaited(false)
{
    worker::Worker::get().setExpectedWork(mWork);
}

ExpectExecutedWork::~ExpectExecutedWork()
{
    ASSERT(mWaited);
}

void ExpectExecutedWork::wait()
{
    ASSERT(!mWaited);
    mWaited = true;
    worker::Worker::get().waitForExecutionCount();
    // Any work done generates at least one event (WorkDoneEvent).
    // Wait until those events have been handled by the main thread.
    waitForIdle();
}

} // namespace