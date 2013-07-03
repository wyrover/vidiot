#include "DialogHelp.h"

#include "Config.h"
#include "UtilLog.h"
#include "Window.h"
#include "UtilLogWxwidgets.h"

namespace gui {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

DialogHelp::DialogHelp()
    :   wxDialog(&Window::get(),wxID_ANY,_("Vidiot: Help"),wxDefaultPosition,wxSize(400,400),wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER,wxDialogNameStr )
    ,   mBack(0)
{
    VAR_DEBUG(this);

    SetSizer(new wxBoxSizer(wxVERTICAL));

    wxString dir = wxFileName(Config::getExeDir() + "\\html\\help\\","").GetFullPath();

    ////////  ////////

    mHtml = new wxHtmlWindow(this);
    mHtml->LoadPage(wxFileName(dir, "index.html").GetFullPath());

    ////////  ////////

    GetSizer()->Add(mHtml, wxSizerFlags(1).Expand());

    ////////  ////////

    wxSizer* buttons = CreateButtonSizer(wxOK);
    ASSERT_NONZERO(buttons);
    mBack = new wxButton(this,wxID_ANY,_("Back"));
    buttons->Add(mBack);
    GetSizer()->Add(buttons);

    mBack->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &DialogHelp::onBack, this);
}

DialogHelp::~DialogHelp()
{

    VAR_DEBUG(this);
    mBack->Unbind(wxEVT_COMMAND_BUTTON_CLICKED, &DialogHelp::onBack, this);
}

void DialogHelp::onBack(wxCommandEvent &event)
{
    mHtml->HistoryBack();
    event.Skip();
}

} //namespace