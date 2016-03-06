// Copyright 2013-2016 Eric Raijmakers.
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

#include "CommandLine.h"
#include "StatusBar.h"
#include "UtilThread.h"
#include "WorkEvent.h"

namespace worker {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

Work::Work(const Callable& work)
    : mCallable(work)
    , mAbort(false)
    , mShowProgress(true)
{
    VAR_DEBUG(*this);
}

Work::~Work()
{
    VAR_DEBUG(this);
}

bool Work::execute(bool showProgress)
{
    VAR_DEBUG(this);
    if (mAbort) { return false; }
    mCallable();
    if (showProgress)
    {
        util::thread::RunInMainScheduler::get().run([]
        {
            // Note that - in the code of mCallable - showProgressText can be called.
            // That method schedules an event that causes the progress bar update.
            // Therefore, here another event is scheduled that resets the text afterwards.
            gui::StatusBar::get().hideProgressBar();
            gui::StatusBar::get().setProcessingText("");
        });
    }
    if (mAbort) { return false; }
    QueueEvent(new WorkDoneEvent(self()));
    VAR_DEBUG(this);
    return true;
}

void Work::stopShowingProgress()
{
    mShowProgress = false;
}

void Work::abort()
{
    mAbort = true;
    stopShowingProgress();
}

//////////////////////////////////////////////////////////////////////////
// ABORT
//////////////////////////////////////////////////////////////////////////

bool Work::isAborted() const
{
    return mAbort;
}

//////////////////////////////////////////////////////////////////////////
// PROGRESS
//////////////////////////////////////////////////////////////////////////

void Work::showProgressText(const wxString& text)
{
    if (mShowProgress)
    {
        gui::StatusBar::get().setProcessingText(text);
    }
}

void Work::showProgressBar(int max, bool taskbar)
{
    if (mShowProgress)
    {
        gui::StatusBar::get().showProgressBar(max, taskbar);
    }
}

void Work::showProgress(int value)
{
    if (mShowProgress)
    {
        gui::StatusBar::get().showProgress(value);
    }
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

void Work::setThreadName(const std::string& name)
{
    std::ostringstream s;
    s << gui::CommandLine::get().ExeName.Mid(0,6) + ":" + name;
    util::thread::setCurrentThreadName(s.str().c_str());
}

std::ostream& operator<<(std::ostream& os, const Work& obj)
{
    os << &obj;
    return os;
}

std::ostream& operator<<(std::ostream& os, const WorkPtr& obj)
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
