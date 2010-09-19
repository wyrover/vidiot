#include "FifoWork.h"

Work::Work(Callable work, wxString description)
:   mCallable(work)
,   mDescription(description)
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

wxString Work::getDescription() const
{
    return mDescription;
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<< (std::ostream& os, const Work& obj)
{
    os << obj.mDescription;
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

