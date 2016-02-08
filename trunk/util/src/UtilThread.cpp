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

#include "UtilThread.h"

#ifdef __GNUC__
#include <sys/prctl.h>
#endif

namespace util { namespace thread {

RunInMainScheduler::RunInMainScheduler()
{
    Bind(wxEVT_THREAD, &RunInMainScheduler::onThreadEvent, this);
}

RunInMainScheduler::~RunInMainScheduler()
{
    Unbind(wxEVT_THREAD, &RunInMainScheduler::onThreadEvent, this);
}

void RunInMainScheduler::run(const std::function<void()>& method)
{
    if (wxThread::IsMain())
    {
        method();
    }
    else
    {
        wxThreadEvent* threadEvent{ new wxThreadEvent{wxEVT_THREAD, wxID_HIGHEST + 1} };
        threadEvent->SetPayload(method);
        QueueEvent(threadEvent);
    }
}

void RunInMainScheduler::onThreadEvent(wxThreadEvent& event)
{
    ASSERT(wxThread::IsMain());
    std::function<void()> method{ event.GetPayload<std::function<void()>>() };
    method();
}

void RunInMainAndWait(const std::function<void()>& method)
{
    std::atomic<bool> done{ false };
    boost::condition_variable condition;
    boost::mutex mutex;
    util::thread::RunInMainScheduler::get().run([method, &done, &mutex, &condition]
    {
        method();
        done = true;
        boost::mutex::scoped_lock lock(mutex);
        condition.notify_all();
    });
    boost::mutex::scoped_lock lock(mutex);
    while (!done)
    {
        condition.wait(lock);
    }
}

void RunInMain(const std::function<void()>& method)
{
    util::thread::RunInMainScheduler::get().run(method);
}

void setCurrentThreadName(const char* name)
{
#if (defined _MSC_VER)
    if (wxIsDebuggerRunning())
    {
        wxThreadIdType id = wxThread::GetCurrentId();

        const DWORD MS_VC_EXCEPTION=0x406D1388;
#pragma pack(push,8)
        typedef struct tagTHREADNAME_INFO
        {
            DWORD dwType; // Must be 0x1000.
            LPCSTR szName; // Pointer to name (in user addr space).
            DWORD dwThreadID; // Thread ID (-1=caller thread).
            DWORD dwFlags; // Reserved for future use, must be zero.
        } THREADNAME_INFO;
#pragma pack(pop)

        THREADNAME_INFO info;
        info.dwType = 0x1000;
        info.szName = name;
        info.dwThreadID = id;
        info.dwFlags = 0;

        __try
        {
            RaiseException( MS_VC_EXCEPTION, 0, sizeof(info)/sizeof(ULONG_PTR), (ULONG_PTR*)&info );
        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
        }
    }
#else
    prctl(PR_SET_NAME,name,0,0,0);
#endif
}

}} // namespace
