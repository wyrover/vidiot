#include "FifoWork.h"

Work::Work(Callable work)
:   mCallable(work)
{
    VAR_DEBUG(this)(*this);
}

Work::~Work()
{
    VAR_DEBUG(this);
}

void Work::execute() const
{
    VAR_DEBUG(this);
    mCallable();
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