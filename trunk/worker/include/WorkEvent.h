#ifndef WORK_EVENT_H
#define WORK_EVENT_H

#include "UtilEvent.h"

namespace worker {

DECLARE_EVENT(EVENT_WORK_DONE, WorkDoneEvent, WorkPtr);

} // namespace

#endif // WORK_EVENT_H