#include "Menu.h"

#include <wx/window.h>
#include <set>
#include <boost/foreach.hpp>
#include <boost/assign/list_of.hpp>
#include "AudioTrack.h"
#include "Clip.h"
#include "CreateAudioTrack.h"
#include "CreateTransition.h"
#include "CreateVideoTrack.h"
#include "EmptyClip.h"
#include "ids.h"
#include "VideoClip.h"
#include "AudioClip.h"
#include "Intervals.h"
#include "MousePointer.h"
#include "PositionInfo.h"
#include "Selection.h"
#include "Sequence.h"
#include "Timeline.h"
#include "TimeLinesView.h"
#include "Track.h"
#include "UtilLog.h"
#include "VideoTrack.h"
#include "Window.h"
#include "Zoom.h"

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

    // Popup menu items
    getTimeline().Bind(wxEVT_COMMAND_MENU_SELECTED,   &MenuHandler::onAddInTransition,    this, meID_ADD_INTRANSITION);

    updateItems();

    Window::get().setSequenceMenu(getMenu());
}

MenuHandler::~MenuHandler()
{
    VAR_DEBUG(this);

    Window::get().Unbind(wxEVT_COMMAND_MENU_SELECTED,    &MenuHandler::onAddVideoTrack,  this, ID_ADDVIDEOTRACK);
    Window::get().Unbind(wxEVT_COMMAND_MENU_SELECTED,    &MenuHandler::onAddAudioTrack,  this, ID_ADDAUDIOTRACK);

    Window::get().Unbind(wxEVT_COMMAND_MENU_SELECTED,    &MenuHandler::onDeleteMarked,   this, ID_DELETEMARKED);
    Window::get().Unbind(wxEVT_COMMAND_MENU_SELECTED,    &MenuHandler::onDeleteUnmarked, this, ID_DELETEUNMARKED);
    Window::get().Unbind(wxEVT_COMMAND_MENU_SELECTED,    &MenuHandler::onRemoveMarkers,  this, ID_REMOVEMARKERS);

    Window::get().Unbind(wxEVT_COMMAND_MENU_SELECTED,    &MenuHandler::onCloseSequence,  this, ID_CLOSESEQUENCE);

    getTimeline().Unbind(wxEVT_COMMAND_MENU_SELECTED,   &MenuHandler::onAddInTransition,   this, meID_ADD_INTRANSITION);

    Window::get().setSequenceMenu(0); // If this is NOT the last timeline to be closed, then an 'activate()' will reset the menu to that other timeline
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

    std::set<model::IClipPtr> selectedClips = getSequence()->getSelectedClips();

    struct MenuOption
    {
        explicit MenuOption(int _id, wxString _text, bool _show = false, bool _enable = true)
            : id(_id)
            , show(_show)
            , enable(_enable)
            , text(_text)
        {}
        int id;
        bool show;
        bool enable;
        wxString text;
        void add(wxMenu& menu)
        {
            if (show)
            {
                menu.Append( id, text );
                menu.Enable( id, enable );
            }
        }
    };

    bool selectedSingleClip = false;
    if ((selectedClips.size() == 1) && (!info.clip->getLink()))
    {
        selectedSingleClip = true;
    }
    if ((selectedClips.size() == 2) && (info.clip->getLink() && info.clip->getLink()->getSelected()))
    {
        selectedSingleClip = true;
    }

    //(info.clip && info.clip->getTrack()->isA<model::VideoTrack>()
    bool clickedOnVideoClip = (info.clip && info.clip->isA<model::VideoClip>());
    bool clickedOnAudioClip = (info.clip && info.clip->isA<model::AudioClip>());
    bool clickedOnEmptyClip = (info.clip && info.clip->isA<model::EmptyClip>());

    bool enableRemoveEmpty = true;

    if (info.onAudioVideoDivider)
    {
    }
    else if (info.onTrackDivider)
    {
    }
    else
    {
        if (info.clip)
        { // todo test with emptyclips
            switch (info.logicalclipposition)
            {
            case TransitionBegin:
            case TransitionLeftClipEnd:
            case TransitionInterior:
            case TransitionRightClipBegin:
            case TransitionEnd:
                break;
            case ClipBegin:
                break;
            case ClipInterior:
                break;
            case ClipEnd:
                break;
            default:
                FATAL("Unexpected logical clip position.");
            }
        }
    }

    wxMenu menu;

    MenuOption addInTransition(meID_ADD_INTRANSITION,   _("Add &in transition"),    clickedOnVideoClip, clickedOnVideoClip);
    MenuOption addOutTransition(meID_ADD_OUTTRANSITION, _("Add &out transition"),   clickedOnVideoClip, clickedOnVideoClip);

    MenuOption addInFade(meID_ADD_INFADE,   _("Add fade &in"),    clickedOnAudioClip, clickedOnAudioClip);
    MenuOption addOutFade(meID_ADD_OUTFADE, _("Add fade &out"),   clickedOnAudioClip, clickedOnAudioClip);

    MenuOption removeEmptySpace(meID_REMOVE_EMPTY, _("&Remove empty space"),   clickedOnEmptyClip, clickedOnEmptyClip);

    addInTransition.add(menu);
    addOutTransition.add(menu);
    addInFade.add(menu);
    addOutFade.add(menu);
    removeEmptySpace.add(menu);
    menu.AppendSeparator();

    //menu.AppendSeparator();
    //pAddMenu = menu.AppendSubMenu(&addMenu,_("&Add"));
    //menu.AppendSeparator();
    //pCreateMenu = menu.AppendSubMenu(&createMenu,_("&New"));
    //menu.UpdateUI(&getWindow());

    getTimeline().PopupMenu(&menu);
}

void MenuHandler::activate()
{
    Window::get().setSequenceMenu(getMenu());
}

//////////////////////////////////////////////////////////////////////////
// SEQUENCE MENU
//////////////////////////////////////////////////////////////////////////

void MenuHandler::onAddVideoTrack(wxCommandEvent& event)
{
    LOG_INFO;
    (new command::CreateVideoTrack(getSequence()))->submit();
}

void MenuHandler::onAddAudioTrack(wxCommandEvent& event)
{
    LOG_INFO;
    (new command::CreateAudioTrack(getSequence()))->submit();
}

void MenuHandler::onDeleteMarked(wxCommandEvent& event)
{
    LOG_INFO;
    getIntervals().deleteMarked();
}

void MenuHandler::onDeleteUnmarked(wxCommandEvent& event)
{
    LOG_INFO;
    getIntervals().deleteUnmarked();
}

void MenuHandler::onRemoveMarkers(wxCommandEvent& event)
{
    LOG_INFO;
    getIntervals().clear();
}

void MenuHandler::onCloseSequence(wxCommandEvent& event)
{
    LOG_INFO;
    TimelinesView& tv = Window::get().getTimeLines();
    tv.Close();
}

//////////////////////////////////////////////////////////////////////////
// POPUP MENU
//////////////////////////////////////////////////////////////////////////

void MenuHandler::onAddInTransition(wxCommandEvent& event)
{
    LOG_INFO;
    command::CreateTransition* cmd = new command::CreateTransition(getSequence(), getMousePointer().getRightDownPosition());
    if (cmd->isPossible())
    {
        cmd->submit();
    }
    else
    {
        delete cmd;
    }
}

}} // namespace