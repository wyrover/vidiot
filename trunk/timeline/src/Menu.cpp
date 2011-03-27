#include "Menu.h"

#include <wx/window.h>
#include "UtilLog.h"
#include "GuiWindow.h"
#include "GuiTimeLinesView.h"
#include "CreateVideoTrack.h"
#include "CreateAudioTrack.h"
#include "PositionInfo.h"
#include "Clip.h"
#include "MousePointer.h"
#include "Intervals.h"
#include "Timeline.h"
#include "ids.h"

namespace gui { namespace timeline {

enum {
    meID_ADD_TRANSITION = wxID_HIGHEST+1,
    meID_REMOVE_EMPTY,
};

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

MenuHandler::MenuHandler(Timeline* timeline)
:   wxEvtHandler()
,   Part(timeline)
,   mMenu()
{
    VAR_DEBUG(this);

    mMenu.Append(ID_ADDVIDEOTRACK,  _("Add video track"));
    mMenu.Append(ID_ADDAUDIOTRACK,  _("Add audio track"));
    mMenu.AppendSeparator();
    mMenu.Append(ID_DELETEMARKED,   _("Delete marked regions from sequence"));
    mMenu.Append(ID_DELETEUNMARKED, _("Delete unmarked regions from sequence"));
    mMenu.Append(ID_REMOVEMARKERS,  _("Remove all markers"));
    mMenu.AppendSeparator();
    mMenu.Append(ID_CLOSESEQUENCE,  _("Close"));

    GuiWindow::get()->Bind(wxEVT_COMMAND_MENU_SELECTED,    &MenuHandler::onAddVideoTrack,  this, ID_ADDVIDEOTRACK);
    GuiWindow::get()->Bind(wxEVT_COMMAND_MENU_SELECTED,    &MenuHandler::onAddAudioTrack,  this, ID_ADDAUDIOTRACK);

    GuiWindow::get()->Bind(wxEVT_COMMAND_MENU_SELECTED,    &MenuHandler::onDeleteMarked,   this, ID_DELETEMARKED);
    GuiWindow::get()->Bind(wxEVT_COMMAND_MENU_SELECTED,    &MenuHandler::onDeleteUnmarked, this, ID_DELETEUNMARKED);
    GuiWindow::get()->Bind(wxEVT_COMMAND_MENU_SELECTED,    &MenuHandler::onRemoveMarkers,  this, ID_REMOVEMARKERS);

    GuiWindow::get()->Bind(wxEVT_COMMAND_MENU_SELECTED,    &MenuHandler::onCloseSequence,  this, ID_CLOSESEQUENCE);

    wxNotebook* notebook = dynamic_cast<wxNotebook*>(getTimeline().GetParent());
    notebook->Bind(wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED,     &MenuHandler::onPageChanged, this);

    updateItems();

    GuiWindow::get()->setSequenceMenu(getMenu());
}

MenuHandler::~MenuHandler()
{
    VAR_DEBUG(this);

    wxNotebook* notebook = dynamic_cast<wxNotebook*>(getTimeline().GetParent());
    notebook->Unbind(wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED,    &MenuHandler::onPageChanged, this);

    GuiWindow::get()->Unbind(wxEVT_COMMAND_MENU_SELECTED,    &MenuHandler::onAddVideoTrack,  this, ID_ADDVIDEOTRACK);
    GuiWindow::get()->Unbind(wxEVT_COMMAND_MENU_SELECTED,    &MenuHandler::onAddAudioTrack,  this, ID_ADDAUDIOTRACK);

    GuiWindow::get()->Unbind(wxEVT_COMMAND_MENU_SELECTED,    &MenuHandler::onDeleteMarked,   this, ID_DELETEMARKED);
    GuiWindow::get()->Unbind(wxEVT_COMMAND_MENU_SELECTED,    &MenuHandler::onDeleteUnmarked, this, ID_DELETEUNMARKED);
    GuiWindow::get()->Unbind(wxEVT_COMMAND_MENU_SELECTED,    &MenuHandler::onRemoveMarkers,  this, ID_REMOVEMARKERS);

    GuiWindow::get()->Unbind(wxEVT_COMMAND_MENU_SELECTED,    &MenuHandler::onCloseSequence,  this, ID_CLOSESEQUENCE);

    GuiWindow::get()->setSequenceMenu(0); // If this is NOT the last timeline to be closed, then a onPageChanged event will reset the menu to that other timeline

}

//////////////////////////////////////////////////////////////////////////
// MENU INTERFACE
//////////////////////////////////////////////////////////////////////////

wxMenu* MenuHandler::getMenu()
{
    return &mMenu;
}

void MenuHandler::updateItems()
{
    mMenu.Enable( ID_DELETEMARKED,   !getIntervals().isEmpty() );
    mMenu.Enable( ID_DELETEUNMARKED, !getIntervals().isEmpty() );
    mMenu.Enable( ID_REMOVEMARKERS,  !getIntervals().isEmpty() );
}

void MenuHandler::Popup()
{
    PointerPositionInfo info = getMousePointer().getInfo(wxGetMouseState().GetPosition());

    // Mechanism:
    // Default menu options are hidden and enabled.
    // If an item is selected for which a menu option makes sense, then the option is shown.
    // If an item is selected for which a menu option does not make sense, then the option is disabled.

    bool showAddTransition = true;
    bool showRemoveEmpty = info.clip && info.clip->isA<model::EmptyClip>();

    bool enableAddTransition = info.clip && !info.clip->isA<model::EmptyClip>();
    bool enableRemoveEmpty = info.clip && info.clip->isA<model::EmptyClip>();

    wxMenu menu;
    menu.Append( meID_ADD_TRANSITION, _("&Add Transition") );
    menu.Enable( meID_ADD_TRANSITION, enableAddTransition );
    menu.Append( meID_REMOVE_EMPTY,   _("&Remove empty space") );
    menu.Enable( meID_REMOVE_EMPTY, enableRemoveEmpty );
    menu.AppendSeparator();

    //menu.AppendSeparator();
    //pAddMenu = menu.AppendSubMenu(&addMenu,_("&Add"));
    //menu.AppendSeparator();
    //pCreateMenu = menu.AppendSubMenu(&createMenu,_("&New"));

    getTimeline().PopupMenu(&menu);
}

//////////////////////////////////////////////////////////////////////////
// SEQUENCE MENU
//////////////////////////////////////////////////////////////////////////

void MenuHandler::onAddVideoTrack(wxCommandEvent& WXUNUSED(event))
{
    LOG_INFO;
    getTimeline().Submit(new command::CreateVideoTrack(getTimeline()));
}

void MenuHandler::onAddAudioTrack(wxCommandEvent& WXUNUSED(event))
{
    LOG_INFO;
    getTimeline().Submit(new command::CreateAudioTrack(getTimeline()));
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
    GuiTimelinesView& tv = GuiWindow::get()->getTimeLines();
    tv.Close();
}

//////////////////////////////////////////////////////////////////////////
// EVENTS
//////////////////////////////////////////////////////////////////////////

void MenuHandler::onPageChanged(wxBookCtrlEvent& event)
{
    wxNotebook* notebook = dynamic_cast<wxNotebook*>(getTimeline().GetParent());
    timeline::Timeline* timeline = static_cast<timeline::Timeline*>(notebook->GetPage(event.GetSelection()));
    if (timeline == &getTimeline())
    {
        GuiWindow::get()->setSequenceMenu(timeline->getMenuHandler().getMenu());
    }
    event.Skip();
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

}} // namespace

