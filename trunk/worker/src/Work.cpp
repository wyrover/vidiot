// Copyright 2013,2014 Eric Raijmakers.
//
// This file is part of Vidiot.
//
// Vidiot is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Vidiot is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Vidiot. If not, see <http://www.gnu.org/licenses/>.

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
    util::thread::RunInMainAndDontWait([]
    {
        // Note that - in the code of mCallable - showProgressText can be called.
        // That method schedules an event that causes the progress bar update.
        // Therefore, here another event is scheduled that resets the text afterwards.
        gui::StatusBar::get().hideProgressBar();
        gui::StatusBar::get().setProcessingText("");
    });
    QueueEvent(new WorkDoneEvent(self()));
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