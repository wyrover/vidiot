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

#include "Help.h"

#include "Config.h"
#include "Preview.h"
#include "ProjectView.h"
#include "TimelinesView.h"
#include "Details.h"
#include "UtilLocale.h"
#include "UtilPath.h"
#include "Window.h"

namespace gui {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

Help::Help(wxWindow* parent)
    :   wxPanel(parent,wxID_ANY,wxDefaultPosition,wxDefaultSize)
    ,   mBack(0)
    ,   mHome(0)
    ,   mForward(0)
    ,   mHighlightTimer(this)
    ,   mHighlight(boost::none)
    ,   mHighlightCount(0)
{
    VAR_DEBUG(this);

    SetSizer(new wxBoxSizer(wxVERTICAL));

    ////////  ////////

    mHtml = new HtmlWindow(this);

    ////////  ////////

    GetSizer()->Add(mHtml, wxSizerFlags(1).Expand());

    ////////  ////////

    wxSizer* buttons = new wxBoxSizer(wxHORIZONTAL);
    ASSERT_NONZERO(buttons);
    mBack = new wxButton(this,wxID_ANY,_("Back"));
    buttons->Add(mBack);
    mHome = new wxButton(this,wxID_ANY,_("Index"));
    buttons->Add(mHome);
    mForward = new wxButton(this,wxID_ANY,_("Forward"));
    buttons->Add(mForward);
    GetSizer()->Add(buttons);

    mBack->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &Help::onBack, this);
    mHome->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &Help::onHome, this);
    mForward->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &Help::onForward, this);
    mHtml->Bind(wxEVT_COMMAND_HTML_LINK_CLICKED , &Help::onLink, this);
    Bind(wxEVT_TIMER, &Help::onTimer, this);

    home();
}

Help::~Help()
{

    VAR_DEBUG(this);
    mHtml->Unbind(wxEVT_COMMAND_HTML_LINK_CLICKED, &Help::onLink, this);
    mHome->Unbind(wxEVT_COMMAND_BUTTON_CLICKED, &Help::onHome, this);
    mBack->Unbind(wxEVT_COMMAND_BUTTON_CLICKED, &Help::onBack, this);
    mForward->Unbind(wxEVT_COMMAND_BUTTON_CLICKED, &Help::onForward, this);
    Unbind(wxEVT_TIMER, &Help::onTimer, this);
}

//////////////////////////////////////////////////////////////////////////
// GUI EVENTS
//////////////////////////////////////////////////////////////////////////

void Help::onLink( wxHtmlLinkEvent& event)
{
    gui::Window::get().getUiManager().HideHint();
    mHighlight.reset();
    if (event.GetLinkInfo().GetHref().IsSameAs("projectview.html"))
    {
        mHighlight.reset(wxRect(gui::ProjectView::get().GetScreenPosition(), gui::ProjectView::get().GetSize()));
    }
    if (event.GetLinkInfo().GetHref().IsSameAs("timeline.html"))
    {
        mHighlight.reset(wxRect(gui::TimelinesView::get().GetScreenPosition(), gui::TimelinesView::get().GetSize()));
    }
    if (event.GetLinkInfo().GetHref().IsSameAs("details.html"))
    {
        mHighlight.reset(wxRect(gui::Window::get().getDetailsView().GetScreenPosition(), gui::Window::get().getDetailsView().GetSize()));
    }
    if (event.GetLinkInfo().GetHref().IsSameAs("preview.html"))
    {
        mHighlight.reset(wxRect(gui::Preview::get().GetScreenPosition(), gui::Preview::get().GetSize()));
    }
    if (mHighlight)
    {
        mHighlightCount = 9; // Odd to show initially
        mHighlightTimer.Start(250, false);
    }
    updateButtons();
    event.Skip();
}

void Help::onBack(wxCommandEvent &event)
{
    mHtml->HistoryBack();
    updateButtons();
    event.Skip();
}

void Help::onHome(wxCommandEvent &event)
{
    home();
    updateButtons();
    event.Skip();
}

void Help::onForward(wxCommandEvent &event)
{
    mHtml->HistoryForward();
    updateButtons();
    event.Skip();
}

void Help::onTimer(wxTimerEvent& event)
{
    gui::Window::get().getUiManager().HideHint();
    if (mHighlightCount % 2 == 0)
    {
        gui::Window::get().getUiManager().HideHint();
    }
    else
    {
        if(mHighlight)
        {
            gui::Window::get().getUiManager().ShowHint(*mHighlight);
        }
    }
    if (mHighlightCount > 0)
    {
        mHighlightCount--;
    }
    event.Skip();
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void Help::home()
{
    wxString main{ util::path::getResource("html/help/" + getLanguageCode(), "index.html") }; // nl_NL
    if (!wxFile::Exists(main))
    {
        main = util::path::getResource("html/help/" + getLanguageCode().Left(2), "index.html"); // nl
    }
    if (!wxFile::Exists(main))
    {
        main = util::path::getResource("html/help/en", "index.html"); // Default: en
    }
    mHtml->LoadPage(main);
}

void Help::updateButtons()
{
    mBack->Enable(mHtml->HistoryCanBack());
    mForward->Enable(mHtml->HistoryCanForward());
}

} //namespace
