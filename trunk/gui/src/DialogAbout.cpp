#include "DialogAbout.h"

#include "Config.h"
#include "UtilLog.h"
#include "Window.h"
#include "UtilLogWxwidgets.h"

namespace gui {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

DialogAbout::DialogAbout()
    :   wxDialog(&Window::get(),wxID_ANY,_("Vidiot: About"),wxDefaultPosition,wxSize(400,400),wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER,wxDialogNameStr )
    ,   mBack(0)
{
    VAR_DEBUG(this);

    SetSizer(new wxBoxSizer(wxVERTICAL));

    wxString dir = wxFileName(Config::getExeDir() + "\\html\\about\\","").GetFullPath();

    ////////  ////////

    mHtml = new wxHtmlWindow(this);
    mHtml->LoadPage(wxFileName(dir, "main.html").GetFullPath());

    ////////  ////////

    GetSizer()->Add(mHtml, wxSizerFlags(1).Expand());

    ////////  ////////

    wxSizer* buttons = CreateButtonSizer(wxOK);
    ASSERT_NONZERO(buttons);
    mBack = new wxButton(this,wxID_ANY,_("Back"));
    buttons->Add(mBack);
    GetSizer()->Add(buttons);

    mBack->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &DialogAbout::onBack, this);
}

DialogAbout::~DialogAbout()
{

    VAR_DEBUG(this);
    mBack->Unbind(wxEVT_COMMAND_BUTTON_CLICKED, &DialogAbout::onBack, this);
}

void DialogAbout::onBack(wxCommandEvent &event)
{
    mHtml->HistoryBack();
    event.Skip();
}

} //namespace