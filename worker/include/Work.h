// Copyright 2013 Eric Raijmakers.
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

#ifndef WORK_H
#define WORK_H

#include "UtilSelf.h"

namespace worker {

typedef boost::function< void() > Callable;

class Work
    : public wxEvtHandler // MUST BE FIRST INHERITED CLASS FOR WXWIDGETS EVENTS TO BE RECEIVED.
    , boost::noncopyable
    , public Self<Work>
{
public:
    explicit Work(Callable work);
    virtual ~Work();

    void execute();

    void abort();

protected:

    bool isAborted() const;

    void showProgressText(wxString text);
    void showProgressBar(int max);
    void showProgress(int value);

    void setThreadName(std::string name);

private:

    Callable mCallable;
    bool mAbort;

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<< (std::ostream& os, const Work& obj);

};

std::ostream& operator<< (std::ostream& os, const WorkPtr& obj);

} // namespace

#endif // WORK_H