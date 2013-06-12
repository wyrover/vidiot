#ifndef WORKER_EVENT_H
#define WORKER_EVENT_H

#include "UtilEvent.h"

namespace worker {

DECLARE_EVENT(EVENT_WORKER_QUEUE_SIZE, WorkerQueueSizeEvent, long);

} // namespace

#endif // WORKER_EVENT_H