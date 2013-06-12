#include "Work.h"

#include "UtilLog.h"
#include "WorkEvent.h"

namespace worker {

Work::Work(Callable work)
:   mCallable(work)
{
    VAR_DEBUG(*this);
}

Work::~Work()
{
    VAR_DEBUG(this);
}

void Work::execute()
{
    VAR_DEBUG(this);
    mCallable();
    QueueEvent(new WorkDoneEvent(shared_from_this()));
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<< (std::ostream& os, const Work& obj)
{
    os << &obj;
    return os;
}

std::ostream& operator<< (std::ostream& os, const WorkPtr& obj)
{
    if (obj)
    {
        os << *obj;
    }
    else
    {
        os << "0";
    }
    return os;
}

} // namespace