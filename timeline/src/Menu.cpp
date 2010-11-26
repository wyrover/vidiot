#include "Menu.h"

#include "UtilLog.h"
#include "GuiMain.h"
#include "GuiWindow.h"
#include "GuiTimeLinesView.h"
#include "Intervals.h"
#include "ids.h"

namespace gui { namespace timeline {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

MenuHandler::MenuHandler(Timeline* timeline)
:   wxEvtHandler()
,   Part(timeline)
,   mMenu()
{
    mMenu.Append(ID_ADDVIDEOTRACK,  _("Add video track"));
    mMenu.Append(ID_ADDAUDIOTRACK,  _("Add audio track"));
    mMenu.AppendSeparator();
    mMenu.Append(ID_DELETEMARKED,   _("Delete marked regions from sequence"));
    mMenu.Append(ID_DELETEUNMARKED, _("Delete unmarked regions from sequence"));
    mMenu.Append(ID_REMOVEMARKERS,  _("Remove all markers"));
    mMenu.AppendSeparator();
    mMenu.Append(ID_CLOSESEQUENCE,  _("Close"));

    wxGetApp().GetTopWindow()->Bind(wxEVT_COMMAND_MENU_SELECTED,    &MenuHandler::onAddVideoTrack,  this, ID_ADDVIDEOTRACK);
    wxGetApp().GetTopWindow()->Bind(wxEVT_COMMAND_MENU_SELECTED,    &MenuHandler::onAddAudioTrack,  this, ID_ADDAUDIOTRACK);

    wxGetApp().GetTopWindow()->Bind(wxEVT_COMMAND_MENU_SELECTED,    &MenuHandler::onDeleteMarked,   this, ID_DELETEMARKED);
    wxGetApp().GetTopWindow()->Bind(wxEVT_COMMAND_MENU_SELECTED,    &MenuHandler::onDeleteUnmarked, this, ID_DELETEUNMARKED);
    wxGetApp().GetTopWindow()->Bind(wxEVT_COMMAND_MENU_SELECTED,    &MenuHandler::onRemoveMarkers,  this, ID_REMOVEMARKERS);

    wxGetApp().GetTopWindow()->Bind(wxEVT_COMMAND_MENU_SELECTED,    &MenuHandler::onCloseSequence,  this, ID_CLOSESEQUENCE);

    update();
}

MenuHandler::~MenuHandler()
{
}

//////////////////////////////////////////////////////////////////////////
// MENU INTERFACE
//////////////////////////////////////////////////////////////////////////

wxMenu* MenuHandler::getMenu()
{
    return &mMenu;
}

void MenuHandler::update()
{
    mMenu.Enable( ID_DELETEMARKED,   !getIntervals().isEmpty() );
    mMenu.Enable( ID_DELETEUNMARKED, !getIntervals().isEmpty() );
    mMenu.Enable( ID_REMOVEMARKERS,  !getIntervals().isEmpty() );
}

//////////////////////////////////////////////////////////////////////////
// SEQUENCE MENU
//////////////////////////////////////////////////////////////////////////

void MenuHandler::onAddVideoTrack(wxCommandEvent& WXUNUSED(event))
{
    LOG_INFO;
    //mProject->Submit(new command::TimelineCreateVideoTrack(*mOpenSequences.begin()));
}

void MenuHandler::onAddAudioTrack(wxCommandEvent& WXUNUSED(event))
{
    LOG_INFO;
    //    mProject->Submit(new command::TimelineCreateAudioTrack(*mOpenSequences.begin()));
}


void MenuHandler::onDeleteMarked(wxCommandEvent& WXUNUSED(event))
{
    LOG_INFO;
    getIntervals().deleteMarked();
}

void MenuHandler::onDeleteUnmarked(wxCommandEvent& WXUNUSED(event))
{
    LOG_INFO;
    getIntervals().deleteUnmarked();
}

void MenuHandler::onRemoveMarkers(wxCommandEvent& WXUNUSED(event))
{
    LOG_INFO;
    getIntervals().clear();
}

void MenuHandler::onCloseSequence(wxCommandEvent& WXUNUSED(event))
{
    LOG_INFO;
    NIY;
    GuiTimelinesView& tv = dynamic_cast<GuiWindow*>(wxGetApp().GetTopWindow())->getTimeLines();
    tv.Close();
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

}} // namespace

