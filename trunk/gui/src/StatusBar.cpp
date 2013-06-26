#include "StatusBar.h"

#include "Config.h"
#include "UtilLog.h"
#include "Worker.h"
#include "WorkerEvent.h"

namespace gui {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

StatusBar::StatusBar(wxWindow *parent)
    :   wxStatusBar(parent,wxID_ANY,wxSTB_DEFAULT_STYLE|wxSTB_ELLIPSIZE_MIDDLE)
{
    SetFieldsCount(getNumberOfStatusBars());
    setDebugText("");
    setProcessingText("");
    setQueueText("");
    mProgress = new wxGauge(this,wxID_ANY,100);
    hideProgressBar();
    Bind(wxEVT_SIZE, &StatusBar::onSize, this);
    worker::Worker::get().Bind(worker::EVENT_WORKER_QUEUE_SIZE, &StatusBar::onWorkerQueueSize, this);
}

StatusBar::~StatusBar()
{
    worker::Worker::get().Unbind(worker::EVENT_WORKER_QUEUE_SIZE, &StatusBar::onWorkerQueueSize, this);
    Unbind(wxEVT_SIZE, &StatusBar::onSize, this);
}

//////////////////////////////////////////////////////////////////////////
// GUI EVENTS
//////////////////////////////////////////////////////////////////////////

void StatusBar::onSize(wxSizeEvent& event)
{
    wxRect bb;
    bool ok = GetFieldRect(Config::getShowDebugInfo() ? 3 : 2,bb);
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
        queuetext = wxString::Format("%d %s", event.getValue(), _("items queued"));
    }
    setQueueText(queuetext);
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

int StatusBar::getNumberOfStatusBars() const
{
    return (Config::getShowDebugInfo() ? 4 : 3);
}

void StatusBar::setDebugText(wxString text)
{
    if (Config::getShowDebugInfo())
    {
        SetStatusText( text, 0 );
    }
}

void StatusBar::setProcessingText(wxString text)
{
    SetStatusText( text, Config::getShowDebugInfo() ? 1 : 0 );
}

void StatusBar::setQueueText(wxString text)
{
    SetStatusText( text, Config::getShowDebugInfo() ? 2 : 1 );
}

void StatusBar::showProgressBar(int max)
{
    mProgress->SetRange(max);
    mProgress->Show();
}

void StatusBar::showProgress(int value)
{
    mProgress->SetValue(value);
}

void StatusBar::hideProgressBar()
{
    mProgress->Hide();
}

} // namespace