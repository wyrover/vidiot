#include "Menu.h"

#include <wx/window.h>
#include <set>
#include <boost/foreach.hpp>
#include <boost/assign/list_of.hpp>
#include "UtilLog.h"
#include "Selection.h"
#include "Window.h"
#include "TimeLinesView.h"
#include "Track.h"
#include "CreateVideoTrack.h"
#include "CreateTransition.h"
#include "CreateAudioTrack.h"
#include "PositionInfo.h"
#include "EmptyClip.h"
#include "Clip.h"
#include "Zoom.h"
#include "MousePointer.h"
#include "Intervals.h"
#include "Timeline.h"
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
    VAR_DEBUG(this);

    mMenu.Append(ID_ADDVIDEOTRACK,  _("Add video track"));
    mMenu.Append(ID_ADDAUDIOTRACK,  _("Add audio track"));
    mMenu.AppendSeparator();
    mMenu.Append(ID_DELETEMARKED,   _("Delete marked regions from sequence"));
    mMenu.Append(ID_DELETEUNMARKED, _("Delete unmarked regions from sequence"));
    mMenu.Append(ID_REMOVEMARKERS,  _("Remove all markers"));
    mMenu.AppendSeparator();
    mMenu.Append(ID_CLOSESEQUENCE,  _("Close"));

    Window::get().Bind(wxEVT_COMMAND_MENU_SELECTED,    &MenuHandler::onAddVideoTrack,  this, ID_ADDVIDEOTRACK);
    Window::get().Bind(wxEVT_COMMAND_MENU_SELECTED,    &MenuHandler::onAddAudioTrack,  this, ID_ADDAUDIOTRACK);

    Window::get().Bind(wxEVT_COMMAND_MENU_SELECTED,    &MenuHandler::onDeleteMarked,   this, ID_DELETEMARKED);
    Window::get().Bind(wxEVT_COMMAND_MENU_SELECTED,    &MenuHandler::onDeleteUnmarked, this, ID_DELETEUNMARKED);
    Window::get().Bind(wxEVT_COMMAND_MENU_SELECTED,    &MenuHandler::onRemoveMarkers,  this, ID_REMOVEMARKERS);

    Window::get().Bind(wxEVT_COMMAND_MENU_SELECTED,    &MenuHandler::onCloseSequence,  this, ID_CLOSESEQUENCE);

    wxNotebook* notebook = dynamic_cast<wxNotebook*>(getTimeline().GetParent());
    notebook->Bind(wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED,     &MenuHandler::onPageChanged, this);

    // Popup menu items
    getTimeline().Bind(wxEVT_COMMAND_MENU_SELECTED,   &MenuHandler::onAddTransition,             this, meID_ADD_TRANSITION);


    updateItems();

    Window::get().setSequenceMenu(getMenu());
}

MenuHandler::~MenuHandler()
{
    VAR_DEBUG(this);

    wxNotebook* notebook = dynamic_cast<wxNotebook*>(getTimeline().GetParent());
    notebook->Unbind(wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED,    &MenuHandler::onPageChanged, this);

    Window::get().Unbind(wxEVT_COMMAND_MENU_SELECTED,    &MenuHandler::onAddVideoTrack,  this, ID_ADDVIDEOTRACK);
    Window::get().Unbind(wxEVT_COMMAND_MENU_SELECTED,    &MenuHandler::onAddAudioTrack,  this, ID_ADDAUDIOTRACK);

    Window::get().Unbind(wxEVT_COMMAND_MENU_SELECTED,    &MenuHandler::onDeleteMarked,   this, ID_DELETEMARKED);
    Window::get().Unbind(wxEVT_COMMAND_MENU_SELECTED,    &MenuHandler::onDeleteUnmarked, this, ID_DELETEUNMARKED);
    Window::get().Unbind(wxEVT_COMMAND_MENU_SELECTED,    &MenuHandler::onRemoveMarkers,  this, ID_REMOVEMARKERS);

    Window::get().Unbind(wxEVT_COMMAND_MENU_SELECTED,    &MenuHandler::onCloseSequence,  this, ID_CLOSESEQUENCE);

    getTimeline().Unbind(wxEVT_COMMAND_MENU_SELECTED,   &MenuHandler::onAddTransition,             this, meID_ADD_TRANSITION);

    Window::get().setSequenceMenu(0); // If this is NOT the last timeline to be closed, then a onPageChanged event will reset the menu to that other timeline

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

void MenuHandler::Popup(wxPoint position)
{
    PointerPositionInfo info = getMousePointer().getInfo(position);

    // Mechanism:
    // Default menu options are hidden and enabled.
    // If an item is selected for which a menu option makes sense, then the option is shown.
    // If an item is selected for which a menu option does not make sense, then the option is disabled.

    std::set<model::IClipPtr> selectedClips = getSelection().getClips();

    bool showAddTransition = false;
    bool showRemoveEmpty = false;

    bool enableAddTransition = info.clip && !info.clip->isA<model::EmptyClip>();
    bool enableRemoveEmpty = true;

    BOOST_FOREACH( model::IClipPtr clip, selectedClips )
    {
        if (clip->isA<model::EmptyClip>())
        {
            showRemoveEmpty = true;
        }
        else
        {
            enableRemoveEmpty = false;
        }
    }


    if (info.onAudioVideoDivider)
    {
    }
    else if (info.onTrackDivider)
    {
    }
    else
    {
        if (info.clip)
        {
            switch (info.logicalclipposition)
            {
            case ClipBegin:
                showAddTransition = true;
                break;
            case ClipInterior:
                break;
            case ClipEnd:
                showAddTransition = true;
                break;
            default:
                FATAL("Unexpected logical clip position.");
            }
        }
    }


    wxMenu menu;
    if (showAddTransition)
    {
        menu.Append( meID_ADD_TRANSITION, _("&Add Transition") );
        menu.Enable( meID_ADD_TRANSITION, enableAddTransition );
    }
    if (showRemoveEmpty)
    {
        menu.Append( meID_REMOVE_EMPTY,   _("&Remove empty space") );
        menu.Enable( meID_REMOVE_EMPTY, enableRemoveEmpty );
    }
    menu.AppendSeparator();

    //menu.AppendSeparator();
    //pAddMenu = menu.AppendSubMenu(&addMenu,_("&Add"));
    //menu.AppendSeparator();
    //pCreateMenu = menu.AppendSubMenu(&createMenu,_("&New"));
    //menu.UpdateUI(&getWindow());

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
    TimelinesView& tv = Window::get().getTimeLines();
    tv.Close();
}


//////////////////////////////////////////////////////////////////////////
// POPUP MENU
//////////////////////////////////////////////////////////////////////////

void MenuHandler::onAddTransition(wxCommandEvent& WXUNUSED(event))
{
    LOG_INFO;
    PointerPositionInfo info = getMousePointer().getInfo(getMousePointer().getRightDownPosition());
    // todo: direct right down after open timeline not received in statealways....
    ASSERT(info.clip)(info);

    model::IClipPtr firstClip;
    model::IClipPtr secondClip;

    model::IClips clips;
    switch (info.logicalclipposition)
    {
    case ClipBegin:
        clips = boost::assign::list_of(info.track->getPreviousClip(info.clip))(info.clip);
        break;
    case ClipInterior:
        break;
    case ClipEnd:
        clips = boost::assign::list_of(info.clip)(info.track->getNextClip(info.clip));
        break;
    default:
        FATAL("Unexpected logical clip position.");
    }

    getTimeline().Submit(new command::CreateTransition(getTimeline(), clips));
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
        Window::get().setSequenceMenu(timeline->getMenuHandler().getMenu());
    }
    event.Skip();
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

}} // namespace

