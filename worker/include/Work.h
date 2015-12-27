// Copyright 2013-2015 Eric Raijmakers.
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

namespace worker {

typedef std::function< void() > Callable;

class Work
    : public wxEvtHandler // MUST BE FIRST INHERITED CLASS FOR WXWIDGETS EVENTS TO BE RECEIVED.
    , public Self<Work>
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    explicit Work(const Callable& work);
    Work(const Work&) = delete;
    Work& operator=(const Work&) = delete;
    virtual ~Work();

    void execute(bool showProgress = true);

    void stopShowingProgress();

    void abort();

protected:

    //////////////////////////////////////////////////////////////////////////
    // ABORT
    //////////////////////////////////////////////////////////////////////////

    bool isAborted() const;

    //////////////////////////////////////////////////////////////////////////
    // PROGRESS
    //////////////////////////////////////////////////////////////////////////

    void showProgressText(const wxString& text);
    void showProgressBar(int max, bool taskbar = false);
    void showProgress(int value);

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    void setThreadName(const std::string& name);

private:

    Callable mCallable;
    bool mAbort;
    bool mShowProgress;

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<<(std::ostream& os, const Work& obj);

};

std::ostream& operator<<(std::ostream& os, const WorkPtr& obj);

} // namespace
