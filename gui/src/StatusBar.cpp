#include "StatusBar.h"

#include "UtilLog.h"
#include "Config.h"

namespace gui {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

static StatusBar* sCurrent = 0;

StatusBar::StatusBar(wxWindow *parent)
    :   wxStatusBar(parent,wxID_ANY,wxSTB_DEFAULT_STYLE)
{
    SetFieldsCount(getNumberOfStatusBars());
    setDebugText(_(""));
    setProcessingText(_(""));
    mProgress = new wxGauge(this,wxID_ANY,100);
    hideProgressBar();
    Bind(wxEVT_SIZE, &StatusBar::onSize, this);
    sCurrent = this;
}

StatusBar::~StatusBar()
{
    sCurrent = 0;
    Unbind(wxEVT_SIZE, &StatusBar::onSize, this);
}

// static
StatusBar& StatusBar::get()
{
    ASSERT(sCurrent);
    return *sCurrent;
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