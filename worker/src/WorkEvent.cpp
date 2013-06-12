#include "WorkEvent.h"

namespace worker {

DEFINE_EVENT(EVENT_WORK_DONE, WorkDoneEvent, WorkPtr);

} // namespace