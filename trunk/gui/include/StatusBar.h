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

#ifndef STATUSBAR_H
#define STATUSBAR_H

#include "UtilSingleInstance.h"

class wxAppProgressIndicator;

namespace worker {
    class Work;
    class Worker;
    class WorkerQueueSizeEvent;
}

namespace gui {

struct Timer;

class StatusBar
    :   public wxStatusBar
    ,   public SingleInstance<StatusBar>
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    StatusBar(wxWindow *parent);
    virtual ~StatusBar();

    //////////////////////////////////////////////////////////////////////////
    // GUI EVENTS
    //////////////////////////////////////////////////////////////////////////

    void onSize(wxSizeEvent& event);

    //////////////////////////////////////////////////////////////////////////
    // WORKER EVENTS
    //////////////////////////////////////////////////////////////////////////

    void onWorkerQueueSize(worker::WorkerQueueSizeEvent& event);

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    int getNumberOfStatusBars() const;

    void setDebugText(const wxString& text = "");

    void pushInfoText(const wxString& text);
    void popInfoText();
    void timedInfoText(const wxString& text);

    void setQueueText(const wxString& text = "");

    void setProcessingText(const wxString& text = "");
    void showProgressBar(int max, bool taskbar);
    void showProgress(int value);
    void hideProgressBar();

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    wxGauge* mProgress;
    Timer* mInfoTimer;
    boost::shared_ptr<wxAppProgressIndicator> mTaskBarProgress;
};

} // namespace

#endif
