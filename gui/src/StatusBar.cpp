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

#include "StatusBar.h"

#include "Config.h"
#include "UtilLog.h"
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
    bool ok = GetFieldRect(sProgress,bb);
    mProgress->SetPosition(bb.GetPosition());
    mProgress->SetSize(bb.GetSize());
}

//////////////////////////////////////////////////////////////////////////
// WORKER EVENTS
//////////////////////////////////////////////////////////////////////////

void StatusBar::onWorkerQueueSize(worker::WorkerQueueSizeEvent& event)
{
    wxString queuetext("");
    if (event.getValue() == 1)
    {
        queuetext = _("1 item queued");
    }
    else if (event.getValue() > 1)
    {
        queuetext = wxString::Format("%ld %s", event.getValue(), _("items queued"));
    }
    setQueueText(queuetext);
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
    ASSERT(wxThread::IsMain());
    if (GetFieldsCount() == sDebug + 1)
    {
        SetStatusText( text, sDebug );
    }
}

void StatusBar::pushInfoText(const wxString& text)
{
    ASSERT(wxThread::IsMain());
    ASSERT(!text.IsSameAs(""));
    PushStatusText( text, sInfo );
}

void StatusBar::popInfoText()
{
    PopStatusText(sInfo);
}

void StatusBar::timedInfoText(const wxString& text)
{
    pushInfoText(text);
    mInfoTimer->StartOnce(5000);
}

void StatusBar::setQueueText(const wxString& text)
{
    ASSERT(wxThread::IsMain());
    SetStatusText( text, sQueue );
}

void StatusBar::setProcessingText(const wxString& text)
{
    ASSERT(wxThread::IsMain());
    SetStatusText( text, sProcessing );
}

void StatusBar::showProgressBar(int max)
{
    ASSERT(wxThread::IsMain());
    mProgress->SetRange(max);
    mProgress->Show();
}

void StatusBar::showProgress(int value)
{
    ASSERT(wxThread::IsMain());
    mProgress->SetValue(value); // todo get crash here in testAddAndRemoveFileToWatchedAutoFolder Test: Add supported but not valid file on disk -- upon indexing the value exceeeds vlength (value was 2, max swas 1 in the failing case)
}

void StatusBar::hideProgressBar()
{
    ASSERT(wxThread::IsMain());
    mProgress->Hide();
}

} // namespace
