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

#include "StatusBar.h"

#include <wx/appprogress.h>
#include "Config.h"
#include "UtilThread.h"
#include "Worker.h"
#include "WorkerEvent.h"

namespace gui {

const int sInfo = 0;
const int sProcessing = 1;
const int sQueue = 2;
const int sProgress = 3;
const int sDebug = 4;

struct Timer : public wxTimer
{
    explicit Timer(StatusBar* owner)
        : wxTimer()
        , mOwner(owner)
    {
    }
    void Notify()
    {
        mOwner->SetStatusText("",sInfo);
    }
    StatusBar* mOwner;
};

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

StatusBar::StatusBar(wxWindow *parent)
    :   wxStatusBar(parent,wxID_ANY,wxSTB_DEFAULT_STYLE|wxSTB_ELLIPSIZE_MIDDLE)
    ,   mInfoTimer(new Timer(this))
{
    SetFieldsCount(getNumberOfStatusBars());
    mProgress = new wxGauge(this,wxID_ANY,100);
    hideProgressBar();
    Bind(wxEVT_SIZE, &StatusBar::onSize, this);
    worker::VisibleWorker::get().Bind(worker::EVENT_WORKER_QUEUE_SIZE, &StatusBar::onWorkerQueueSize, this);
}

StatusBar::~StatusBar()
{
    delete mInfoTimer;
    worker::VisibleWorker::get().Unbind(worker::EVENT_WORKER_QUEUE_SIZE, &StatusBar::onWorkerQueueSize, this);
    Unbind(wxEVT_SIZE, &StatusBar::onSize, this);
}

//////////////////////////////////////////////////////////////////////////
// GUI EVENTS
//////////////////////////////////////////////////////////////////////////

void StatusBar::onSize(wxSizeEvent& event)
{
    wxRect bb;
    GetFieldRect(sProgress,bb);
    mProgress->SetPosition(bb.GetPosition());
    mProgress->SetSize(bb.GetSize());
}

//////////////////////////////////////////////////////////////////////////
// WORKER EVENTS
//////////////////////////////////////////////////////////////////////////

void StatusBar::onWorkerQueueSize(worker::WorkerQueueSizeEvent& event)
{
    if (event.getValue() == 0)
    {
        setQueueText("");
    }
    else
    {
        setQueueText(wxString::Format(_("%d item(s) queued"), event.getValue()));
    }
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

int StatusBar::getNumberOfStatusBars() const
{
    ASSERT(wxThread::IsMain());
    return (Config::getShowDebugInfo() ? 5 : 4);
}

void StatusBar::setDebugText(const wxString& text)
{
    util::thread::RunInMainScheduler::get().run([this, text]
    {
        if (GetFieldsCount() == sDebug + 1)
        {
            SetStatusText(text, sDebug);
        }
    });
}

void StatusBar::pushInfoText(const wxString& text)
{
    util::thread::RunInMainScheduler::get().run([this, text]
    {
        ASSERT(!text.IsSameAs(""));
        PushStatusText(text, sInfo);
    });
}

void StatusBar::popInfoText()
{
    util::thread::RunInMainScheduler::get().run([this]
    {
        PopStatusText(sInfo);
    });
}

void StatusBar::timedInfoText(const wxString& text, int ms)
{
    util::thread::RunInMainScheduler::get().run([this, text, ms]
    {
        pushInfoText(text);
        mInfoTimer->StartOnce(ms);
    });
}

void StatusBar::setQueueText(const wxString& text)
{
    util::thread::RunInMainScheduler::get().run([this, text]
    {
        SetStatusText(text, sQueue);
    });
}

void StatusBar::setProcessingText(const wxString& text)
{
    util::thread::RunInMainScheduler::get().run([this, text]
    {
        SetStatusText(text, sProcessing);
    });
}

void StatusBar::showProgressBar(int max, bool taskbar)
{
    util::thread::RunInMainScheduler::get().run([this, max, taskbar]
    {
        mTaskBarProgress.reset(); // Remove any pending progress bar indicator.
        if (taskbar)
        {
            mTaskBarProgress = boost::make_shared<wxAppProgressIndicator>(GetParent(), max);
        }
        mProgress->SetRange(max);
        mProgress->Show();
    });
}

void StatusBar::showProgress(int value)
{
    util::thread::RunInMainScheduler::get().run([this, value]
    {
        if (mProgress->GetRange() < value)
        {
            // Got crash here occasionally in some tests.
            // Maybe caused by initially setting the max to 'the number of files to be index' in an AutoFolder.
            // However, during the indexed (or, directly after setting the max) a new file may be added in the
            // folder causing the number of items to exceed the original max value.
            mProgress->SetRange(value);
            if (mTaskBarProgress)
            {
                mTaskBarProgress->SetRange(value);
            }
        }
        mProgress->SetValue(value);

        if (mTaskBarProgress)
        {
            mTaskBarProgress->SetValue(value);
        }
    });
}

void StatusBar::hideProgressBar()
{
    util::thread::RunInMainScheduler::get().run([this]
    {
        mProgress->Hide();
        mTaskBarProgress.reset();
    });
}

} // namespace
