#include "Work.h"

#include "StatusBar.h"
#include "UtilLog.h"
#include "UtilThread.h"
#include "WorkEvent.h"

namespace worker {

Work::Work(Callable work)
:   mCallable(work)
,   mAbort(false)
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
    if (!mAbort)
    {
        util::thread::RunInMainAndDontWait([]
        {
            gui::StatusBar::get().hideProgressBar();
            gui::StatusBar::get().setProcessingText("");
        });
    }
    QueueEvent(new WorkDoneEvent(shared_from_this()));
    VAR_DEBUG(this);
}

void Work::abort()
{
    mAbort = true;
}

bool Work::isAborted() const
{
    return mAbort;
}

void Work::showProgressText(wxString text)
{
    if (!mAbort)
    {
        util::thread::RunInMainAndDontWait([text]
        {
            gui::StatusBar::get().setProcessingText(text);
        });
    }
}

void Work::showProgressBar(int max)
{
    if (!mAbort)
    {
        util::thread::RunInMainAndDontWait([max]
        {
            gui::StatusBar::get().showProgressBar(max);
        });

    }
}

void Work::showProgress(int value)
{
    if (!mAbort)
    {
        util::thread::RunInMainAndDontWait([value]
        {
            gui::StatusBar::get().showProgress(value);
        });
    }
}

void Work::setThreadName(std::string name)
{
    std::ostringstream s; s << "Worker: " + name;
    util::thread::setCurrentThreadName(s.str().c_str());
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