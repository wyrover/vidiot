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

#include "DialogAbout.h"

#include "Application.h"
#include "Config.h"
#include "UtilLog.h"
#include "UtilLogWxwidgets.h"
#include "UtilPath.h"
#include "UtilWindow.h"
#include "Window.h"

namespace gui {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
wxString sDebug(" (Debug)");
#else
wxString sDebug("");
#endif

DialogAbout::DialogAbout()
    :   wxDialog(&Window::get(),wxID_ANY,_("Vidiot: About") + wxString::Format(wxT(" (%s - %d)%s"), Application::getVersion(), Application::getRevision(), sDebug),wxDefaultPosition,wxSize(500,500),wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER,wxDialogNameStr )
    ,   mBack(0)
{
    VAR_DEBUG(this);

    util::window::setIcons(this);

    SetSizer(new wxBoxSizer(wxVERTICAL));

    ////////  ////////

    mHtml = new wxHtmlWindow(this);
    mHtml->LoadPage(util::path::toFileInInstallationDirectory("html\\about", "main.html"));

    ////////  ////////

    GetSizer()->Add(mHtml, wxSizerFlags(1).Expand());

    ////////  ////////

    wxSizer* buttons = CreateButtonSizer(wxOK);
    ASSERT_NONZERO(buttons);
    mBack = new wxButton(this,wxID_ANY,_("Back"));
    buttons->Add(mBack);
    GetSizer()->Add(buttons);

    mHtml->Bind(wxEVT_HTML_LINK_CLICKED, &DialogAbout::onLink, this);
    mBack->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &DialogAbout::onBack, this);
}

DialogAbout::~DialogAbout()
{

    VAR_DEBUG(this);
    mHtml->Unbind(wxEVT_HTML_LINK_CLICKED, &DialogAbout::onLink, this);
    mBack->Unbind(wxEVT_COMMAND_BUTTON_CLICKED, &DialogAbout::onBack, this);
}

void DialogAbout::onLink(wxHtmlLinkEvent& event)
{
    if (event.GetLinkInfo().GetHref().StartsWith(_T("http://")) || 
        event.GetLinkInfo().GetHref().StartsWith(_T("https://")))
    {
		wxLaunchDefaultBrowser(event.GetLinkInfo().GetHref());
        event.Skip(false);
    }
    else
    {
        event.Skip(true);
    }
}

void DialogAbout::onBack(wxCommandEvent &event)
{
    mHtml->HistoryBack();
    event.Skip();
}

} //namespace
