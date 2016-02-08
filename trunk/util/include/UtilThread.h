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

#pragma once

namespace util { namespace thread {

struct RunInMainScheduler
    : public wxEvtHandler // MUST BE FIRST INHERITED CLASS FOR WXWIDGETS EVENTS TO BE RECEIVED.
    , public SingleInstance<RunInMainScheduler>
{
    RunInMainScheduler();
    RunInMainScheduler(const RunInMainScheduler&) = delete;
    RunInMainScheduler& operator=(const RunInMainScheduler&) = delete;
    ~RunInMainScheduler();
    void run(const std::function<void()>& method);

private:

    void onThreadEvent(wxThreadEvent& event);
};

void RunInMain(const std::function<void()> &method);
void RunInMainAndWait(const std::function<void()> &method);

template <typename RETURNTYPE>
RETURNTYPE RunInMainReturning(const std::function<RETURNTYPE()>& method)
{
    RETURNTYPE result;
    RunInMainAndWait([method, &result]
    {
        result = method();
    });
    return result;
};

void setCurrentThreadName(const char* name);

}} // namespace
