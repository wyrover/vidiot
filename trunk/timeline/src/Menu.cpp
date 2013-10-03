// Copyright 2013 Eric Raijmakers.
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

#include "Menu.h"

#include "AudioClip.h"
#include "AudioTrack.h"
#include "AudioTransitionFactory.h"
#include "Clip.h"
#include "Combiner.h"
#include "Config.h"
#include "CreateAudioTrack.h"
#include "CreateTransitionHelper.h"
#include "CreateVideoTrack.h"
#include "DialogRenderSettings.h"
#include "EmptyClip.h"
#include "ids.h"
#include "Intervals.h"
#include "Mouse.h"
#include "PositionInfo.h"
#include "RemoveEmptyTracks.h"
#include "Render.h"
#include "Selection.h"
#include "Sequence.h"
#include "SplitAtCursor.h"
#include "StatusBar.h"
#include "Timeline.h"
#include "TimeLinesView.h"
#include "Track.h"
#include "Transition.h"
#include "TrimClip.h"
#include "UnlinkClips.h"
#include "UtilLog.h"
#include "VideoClip.h"
#include "VideoDisplayEvent.h"
#include "VideoTrack.h"
#include "VideoTransitionFactory.h"
#include "Window.h"
#include "Zoom.h"

namespace gui { namespace timeline {

enum
{
    ID_TRIGGER_POPUP_MENU = wxID_HIGHEST + 1,
    ID_ADD_INTRANSITION,
    ID_ADD_INOUTTRANSITION,
    ID_ADD_OUTTRANSITION,
    ID_ADD_OUTINTRANSITION,
    ID_REMOVE_EMPTY,
    ID_DELETE_CLIPS,
    ID_DELETE_TRIM_CLIPS,
    ID_UNLINK_CLIPS,
    ID_POPUP_END
};

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

MenuHandler::MenuHandler(Timeline* timeline)
    :   wxEvtHandler()
    ,   Part(timeline)
    ,   mMenu()
    ,   mActive(true)
    ,   mPopup(false)
{
    VAR_DEBUG(this);

    mMenu.Append(ID_ADDVIDEOTRACK,  _("Add video track"),  _("Add a new video track to the sequence."));
    mMenu.Append(ID_ADDAUDIOTRACK,  _("Add audio track"),  _("Add a new audio track to the sequence."));
    mMenu.Append(ID_REMOVE_EMPTY_TRACKS,  _("Remove empty tracks"), _("Remove all empty audio and video tracks in this sequence."));
    mMenu.AppendSeparator();
    mMenu.Append(ID_SPLIT_AT_CURSOR,   _("Split at cursor\tS"), _("Split clips at the current cursor position."));
    mMenu.AppendSeparator();
    mMenu.Append(ID_DELETEMARKED,   _("Delete marked regions"), _("Delete all marked regions from sequence."));
    mMenu.Append(ID_DELETEUNMARKED, _("Delete unmarked regions"), _("Delete all unmarked regions from sequence."));
    mMenu.Append(ID_REMOVEMARKERS,  _("Remove markers"), _("Rmove all markers from the sequence."));
    mMenu.AppendSeparator();
    mMenu.Append(ID_DELETEEMPTY,  _("Remove empty"), _("Remove all empty areas from the sequence."));
    mMenu.AppendSeparator();
    mMenu.Append(ID_RENDERSETTINGS, _("Render settings"), ("Open the dialog containing the settings for generating a movie file from the sequence."));
    mMenu.Append(ID_RENDERSEQUENCE, _("Render '") + getSequence()->getName() + "'", _("Generate movie file from sequence."));
    mMenu.Append(ID_RENDERSEQUENCE, _("Render all modified sequences"), _("Generate movie files for all sequences in the project."));
    mMenu.AppendSeparator();
    mMenu.Append(ID_CLOSESEQUENCE,  _("Close"), _("Close the sequence. Will not remove sequence from project."));

    Window::get().Bind(wxEVT_COMMAND_MENU_SELECTED,    &MenuHandler::onAddVideoTrack,         this, ID_ADDVIDEOTRACK);
    Window::get().Bind(wxEVT_COMMAND_MENU_SELECTED,    &MenuHandler::onAddAudioTrack,         this, ID_ADDAUDIOTRACK);
    Window::get().Bind(wxEVT_COMMAND_MENU_SELECTED,    &MenuHandler::onRemoveEmptyTracks,     this, ID_REMOVE_EMPTY_TRACKS);

    Window::get().Bind(wxEVT_COMMAND_MENU_SELECTED,    &MenuHandler::onSplitAtCursor,         this, ID_SPLIT_AT_CURSOR);

    Window::get().Bind(wxEVT_COMMAND_MENU_SELECTED,    &MenuHandler::onDeleteMarked,          this, ID_DELETEMARKED);
    Window::get().Bind(wxEVT_COMMAND_MENU_SELECTED,    &MenuHandler::onDeleteUnmarked,        this, ID_DELETEUNMARKED);
    Window::get().Bind(wxEVT_COMMAND_MENU_SELECTED,    &MenuHandler::onRemoveMarkers,         this, ID_REMOVEMARKERS);

    Window::get().Bind(wxEVT_COMMAND_MENU_SELECTED,    &MenuHandler::onRemoveAllEmpty,        this, ID_DELETEEMPTY);

    Window::get().Bind(wxEVT_COMMAND_MENU_SELECTED,    &MenuHandler::onRenderSettings,        this, ID_RENDERSETTINGS);
    Window::get().Bind(wxEVT_COMMAND_MENU_SELECTED,    &MenuHandler::onRenderSequence,        this, ID_RENDERSEQUENCE);
    Window::get().Bind(wxEVT_COMMAND_MENU_SELECTED,    &MenuHandler::onRenderAll,             this, ID_RENDERALL);

    Window::get().Bind(wxEVT_COMMAND_MENU_SELECTED,    &MenuHandler::onCloseSequence,         this, ID_CLOSESEQUENCE);

    getTimeline().Bind(wxEVT_COMMAND_MENU_SELECTED,    &MenuHandler::onTriggerPopupMenu,      this, ID_TRIGGER_POPUP_MENU);

    getPlayer()->Bind(EVENT_PLAYBACK_ACTIVE,           &MenuHandler::onPlaybackActive,        this);

    updateItems();

    Window::get().setSequenceMenu(getMenu(), true);
}

MenuHandler::~MenuHandler()
{
    VAR_DEBUG(this);

    Window::get().Unbind(wxEVT_COMMAND_MENU_SELECTED,    &MenuHandler::onAddVideoTrack,         this, ID_ADDVIDEOTRACK);
    Window::get().Unbind(wxEVT_COMMAND_MENU_SELECTED,    &MenuHandler::onAddAudioTrack,         this, ID_ADDAUDIOTRACK);
    Window::get().Unbind(wxEVT_COMMAND_MENU_SELECTED,    &MenuHandler::onRemoveEmptyTracks,     this, ID_REMOVE_EMPTY_TRACKS);

    Window::get().Bind(wxEVT_COMMAND_MENU_SELECTED,      &MenuHandler::onSplitAtCursor,         this, ID_SPLIT_AT_CURSOR);

    Window::get().Unbind(wxEVT_COMMAND_MENU_SELECTED,    &MenuHandler::onDeleteMarked,          this, ID_DELETEMARKED);
    Window::get().Unbind(wxEVT_COMMAND_MENU_SELECTED,    &MenuHandler::onDeleteUnmarked,        this, ID_DELETEUNMARKED);
    Window::get().Unbind(wxEVT_COMMAND_MENU_SELECTED,    &MenuHandler::onRemoveMarkers,         this, ID_REMOVEMARKERS);

    Window::get().Unbind(wxEVT_COMMAND_MENU_SELECTED,    &MenuHandler::onRemoveAllEmpty,        this, ID_DELETEEMPTY);

    Window::get().Unbind(wxEVT_COMMAND_MENU_SELECTED,    &MenuHandler::onRenderSettings,        this, ID_RENDERSETTINGS);
    Window::get().Unbind(wxEVT_COMMAND_MENU_SELECTED,    &MenuHandler::onRenderSequence,        this, ID_RENDERSEQUENCE);
    Window::get().Unbind(wxEVT_COMMAND_MENU_SELECTED,    &MenuHandler::onRenderAll,             this, ID_RENDERALL);

    Window::get().Unbind(wxEVT_COMMAND_MENU_SELECTED,    &MenuHandler::onCloseSequence,         this, ID_CLOSESEQUENCE);

    getTimeline().Unbind(wxEVT_COMMAND_MENU_SELECTED,    &MenuHandler::onTriggerPopupMenu,      this, ID_TRIGGER_POPUP_MENU);

    getPlayer()->Unbind(EVENT_PLAYBACK_ACTIVE,           &MenuHandler::onPlaybackActive,        this);

    Window::get().setSequenceMenu(0, false); // If this is NOT the last timeline to be closed, then an 'activate()' will reset the menu to that other timeline
}

//////////////////////////////////////////////////////////////////////////
// NON MENU EVENTS
//////////////////////////////////////////////////////////////////////////

void MenuHandler::onPlaybackActive(PlaybackActiveEvent& event)
{
    LOG_DEBUG;
    // Disable the menu if the playback is active.
    // Note that enabling the menu options can cause problems:
    // For instance, pressing 's' will call ' MenuHandler::onSplitAtCursor'
    // which causes a change to the sequence while playback is still active!
    Window::get().setSequenceMenu(getMenu(), !event.getValue());
    // todo make a more robust mechanism in the timeline that discards events, given the current state (maybe all actions should be done via the state machine...)
    event.Skip();
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
    // This is decoupled via an event, to ensure that this method returns immediately.
    // Otherwise, the state chart handling is blocked for a while (duration of popup) causing crashes in boost statechart.
    getTimeline().GetEventHandler()->QueueEvent(new wxCommandEvent(wxEVT_COMMAND_MENU_SELECTED,ID_TRIGGER_POPUP_MENU));
}

void MenuHandler::onTriggerPopupMenu(wxCommandEvent& event)
{
    ASSERT(mActive);
    LOG_INFO;

    // Mechanism:
    // Default menu options are hidden and enabled.
    // If an item is selected for which a menu option makes sense, then the option is shown.
    // If an item is selected for which a menu option does not make sense, then the option is disabled.

    wxMenu menu;
    wxMenu* menuFadeIn = new wxMenu; // On heap, destroyed when toplevel menu destroyed
    wxMenu* menuFadeOut = new wxMenu;
    wxMenu* menuFadeInOut = new wxMenu;
    wxMenu* menuFadeOutIn = new wxMenu;

    auto add = [&menu](wxMenu& menu, int id, wxString text, bool show, bool enable, bool separate)
    {
        if (show)
        {
            if (separate)
            {
                menu.AppendSeparator();
            }
            menu.Append( id, text );
            menu.Enable( id, enable );
        }
    };

    bool selectedEmptyClip = false;
    bool selectedMediaClip = false;
    std::set< model::IClipPtr > selection = getSequence()->getSelectedClips();
    BOOST_FOREACH( model::IClipPtr selectedClip, selection )
    {
        if (selectedClip->isA<model::EmptyClip>()) { selectedEmptyClip = true; }
        if (selectedClip->isA<model::VideoClip>()) { selectedMediaClip = true; }
        if (selectedClip->isA<model::AudioClip>()) { selectedMediaClip = true; }
    }

    mPopupPosition = getMouse().getRightDownPosition();
    PointerPositionInfo info = getMouse().getInfo(mPopupPosition);

    model::IClipPtr clickedClip = info.getLogicalClip();
    bool clickedOnVideoClip = (clickedClip && clickedClip->isA<model::VideoClip>());
    bool clickedOnAudioClip = (clickedClip && clickedClip->isA<model::AudioClip>());
    bool clickedOnEmptyClip = (clickedClip && clickedClip->isA<model::EmptyClip>());
    bool clickedOnMediaClip = clickedOnVideoClip || clickedOnAudioClip;

    bool canFadeIn = true;
    bool canFadeOut = true;
    bool canFadeToNext = true;
    bool canFadeFromPrevious = true;

    if (clickedClip)
    {
        // Check if there's already a transition, disabling the transition menu options
        model::TransitionPtr prevTransition = boost::dynamic_pointer_cast<model::Transition>(clickedClip->getPrev());
        if (prevTransition && prevTransition->getRight() > 0)
        {
            canFadeIn = false;
            canFadeFromPrevious = false;
        }
        model::TransitionPtr nextTransition = boost::dynamic_pointer_cast<model::Transition>(clickedClip->getNext());
        if (nextTransition && nextTransition->getLeft() > 0)
        {
            canFadeOut = false;
            canFadeToNext = false;
        }

        // Check if the clicked clip has room for making a crossfade
        if (clickedClip->getMaxAdjustBegin() <= 0)
        {
            canFadeFromPrevious = false;
        }
        if (clickedClip->getMinAdjustEnd() >= 0)
        {
            canFadeToNext = false;
        }

        // Check if the adjacent clip has room for making a crossfade
        if (!clickedClip->getPrev() || clickedClip->getPrev()->getMinAdjustEnd() >= 0)
        {
            canFadeFromPrevious = false;
        }
        if (!clickedClip->getNext() || clickedClip->getNext()->getMaxAdjustBegin() <= 0)
        {
            canFadeToNext = false;
        }
    }

    bool enableUnlink = false;
    model::IClips unlink;
    if (selection.size() == 2)
    {
        std::set< model::IClipPtr >::iterator it = selection.begin();
        model::IClipPtr clip1 = *it++;
        model::IClipPtr clip2 = *it;
        if (clip1->getLink() == clip2)
        {
            ASSERT_EQUALS(clip1,clip2->getLink());
            unlink.push_back(clip1);
            unlink.push_back(clip2);
            enableUnlink = true;
        }
    }

    if (clickedOnAudioClip || clickedOnVideoClip)
    {
        add(menu, ID_ADD_INTRANSITION,    _("Fade &in"),                              clickedOnMediaClip, canFadeIn,              false);
        add(menu, ID_ADD_OUTTRANSITION,   _("Fade &out"),                             clickedOnMediaClip, canFadeOut,             false);
        add(menu, ID_ADD_INOUTTRANSITION, _("Cross-fade from &previous"),             clickedOnMediaClip, canFadeFromPrevious,    false);
        add(menu, ID_ADD_OUTINTRANSITION, _("Cross-fade to &next"),                   clickedOnMediaClip, canFadeToNext,          false);
    }
    std::map<int, model::TransitionType> mapMenuItemToTransitionType;
    std::map<int, model::TransitionDescription> mapMenuItemToTransitionDescription;
    if (clickedOnVideoClip) // For audio clips there is only the crossfade
    {
        int id = ID_POPUP_END;

        BOOST_FOREACH( model::TransitionDescription t, model::video::VideoTransitionFactory::get().getAllPossibleTransitions() )
        {
            add(*menuFadeIn, id, t.second,  clickedOnMediaClip, canFadeIn, false);
            mapMenuItemToTransitionType[id] = model::TransitionTypeIn;
            mapMenuItemToTransitionDescription[id] = t;
            id++;

            add(*menuFadeOut, id, t.second,  clickedOnMediaClip, canFadeOut, false);
            mapMenuItemToTransitionType[id] = model::TransitionTypeOut;
            mapMenuItemToTransitionDescription[id] = t;
            id++;

            add(*menuFadeInOut, id, t.second,  clickedOnMediaClip, canFadeFromPrevious, false);
            mapMenuItemToTransitionType[id] = model::TransitionTypeInOut;
            mapMenuItemToTransitionDescription[id] = t;
            id++;

            add(*menuFadeOutIn, id, t.second,  clickedOnMediaClip, canFadeToNext, false);
            mapMenuItemToTransitionType[id] = model::TransitionTypeOutIn;
            mapMenuItemToTransitionDescription[id] = t;
            id++;
        }
        menu.AppendSeparator();
        menu.AppendSubMenu(menuFadeIn,     _("More fade in"),              _("Show fade in transitions"));
        menu.AppendSubMenu(menuFadeOut,    _("More fade out"),             _("Show fade out transitions"));
        menu.AppendSubMenu(menuFadeInOut,  _("More fade from previous"),   _("Show fade from previous transitions"));
        menu.AppendSubMenu(menuFadeOutIn,  _("More fade to next"),         _("Show fade to next transitions"));
    }

    add(menu, ID_REMOVE_EMPTY,        _("Remove &empty space"),                   clickedOnEmptyClip, clickedOnEmptyClip,     true);
    add(menu, ID_DELETE_CLIPS,        _("&Delete selected\tDel"),                 selectedMediaClip,  !selectedEmptyClip,     true);
    add(menu, ID_DELETE_TRIM_CLIPS,   _("Delete and &Trim selected\tShift+Del"),  selectedMediaClip,  !selectedEmptyClip,     false);
    add(menu, ID_UNLINK_CLIPS,        _("&Unlink audio and video clips"),         selectedMediaClip,  enableUnlink,           true);

    if (menu.GetMenuItemCount() > 0)
    {
        mPopup = true;

        model::TransitionPtr defaultTransition = clickedOnVideoClip ? model::video::VideoTransitionFactory::get().getDefault() : model::audio::AudioTransitionFactory::get().getDefault();
        ASSERT(info.clip);

        int result = getTimeline().GetPopupMenuSelectionFromUser(menu);
        mPopup = false;
        switch (result)
        {
        case wxID_NONE:
            break;
        case ID_ADD_INTRANSITION:
            command::createTransition(getSequence(), info.getLogicalClip(), model::TransitionTypeIn, defaultTransition );
            break;
        case ID_ADD_OUTTRANSITION:
            command::createTransition(getSequence(), info.getLogicalClip(), model::TransitionTypeOut, defaultTransition );
            break;
        case ID_ADD_INOUTTRANSITION:
            command::createTransition(getSequence(), info.getLogicalClip(), model::TransitionTypeInOut, defaultTransition);
            break;
        case ID_ADD_OUTINTRANSITION:
            command::createTransition(getSequence(), info.getLogicalClip(), model::TransitionTypeOutIn, defaultTransition);
            break;
        case ID_REMOVE_EMPTY:
            getIntervals().deleteEmptyClip(info.clip);
            break;
        case ID_DELETE_CLIPS:
            getSelection().deleteClips();
            break;
        case ID_DELETE_TRIM_CLIPS:
            getSelection().deleteClips(true);
            break;
        case ID_UNLINK_CLIPS:
            (new command::UnlinkClips(getSequence(),unlink))->submit();
            break;
        default:
            ASSERT(clickedOnVideoClip);
            ASSERT_MORE_THAN_EQUALS(result, ID_POPUP_END); // Selected one of the video transitions
            ASSERT_MAP_CONTAINS(mapMenuItemToTransitionType,result);
            ASSERT_MAP_CONTAINS(mapMenuItemToTransitionDescription,result);
            model::TransitionPtr transition = model::video::VideoTransitionFactory::get().getTransition(mapMenuItemToTransitionDescription[result]);
            command::createTransition(getSequence(), info.getLogicalClip(), mapMenuItemToTransitionType[result], transition );
            break;
        }
    }
}

void MenuHandler::activate(bool active)
{
    mActive = active;
    if (mActive)
    {
        Window::get().setSequenceMenu(getMenu(),true);
    }
}

bool MenuHandler::isPopupShown() const
{
    return mPopup;
}

//////////////////////////////////////////////////////////////////////////
// SEQUENCE MENU
//////////////////////////////////////////////////////////////////////////

void MenuHandler::onAddVideoTrack(wxCommandEvent& event)
{
    if (mActive)
    {
        LOG_INFO;
        (new command::CreateVideoTrack(getSequence()))->submit();
    }
    event.Skip();
}

void MenuHandler::onAddAudioTrack(wxCommandEvent& event)
{
    if (mActive)
    {
        LOG_INFO;
        (new command::CreateAudioTrack(getSequence()))->submit();
    }
    event.Skip();
}

void MenuHandler::onRemoveEmptyTracks(wxCommandEvent& event)
{
    if (mActive)
    {
        LOG_INFO;
        (new command::RemoveEmptyTracks(getSequence()))->submit();
    }
    event.Skip();
}

void MenuHandler::onSplitAtCursor(wxCommandEvent& event)
{
    if (mActive)
    {
        LOG_INFO;
        (new command::SplitAtCursor(getSequence()))->submit();
    }
    event.Skip();
}

void MenuHandler::onDeleteMarked(wxCommandEvent& event)
{
    if (mActive)
    {
        LOG_INFO;
        getIntervals().deleteMarked();
    }
    event.Skip();
}

void MenuHandler::onDeleteUnmarked(wxCommandEvent& event)
{
    if (mActive)
    {
        LOG_INFO;
        getIntervals().deleteUnmarked();
    }
    event.Skip();
}

void MenuHandler::onRemoveMarkers(wxCommandEvent& event)
{
    if (mActive)
    {
        LOG_INFO;
        getIntervals().clear();
    }
    event.Skip();
}

void MenuHandler::onRemoveAllEmpty(wxCommandEvent& event)
{
    if (mActive)
    {
        LOG_INFO;
        getIntervals().deleteEmpty();
    }
    event.Skip();
}

void MenuHandler::onDeleteSelectedClips(wxCommandEvent& event)
{
    if (mActive)
    {
        LOG_INFO;
        getSelection().deleteClips();
    }
    event.Skip();
}

void MenuHandler::onRenderSettings(wxCommandEvent& event)
{
    if (mActive)
    {
        DialogRenderSettings(getSequence()).ShowModal();
    }
    event.Skip();
}

void MenuHandler::onRenderSequence(wxCommandEvent& event)
{
    if (mActive)
    {
        if (!getSequence()->getRender()->checkFileName())
        {
            DialogRenderSettings(getSequence()).ShowModal();
        }
        else
        {
            model::render::Render::schedule(getSequence());
        }
    }
    event.Skip();
}

void MenuHandler::onRenderAll(wxCommandEvent& event)
{
    // Done, regardless of the active timeline
    model::render::Render::scheduleAll();
    event.Skip();
}

void MenuHandler::onCloseSequence(wxCommandEvent& event)
{
    if (mActive)
    {
        LOG_INFO;
        TimelinesView& tv = Window::get().getTimeLines();
        tv.Close();
        // NOT: event.Skip(); - Causes crash, since the originating menu has been removed.
    }
    else
    {
        event.Skip();
    }
}

}} // namespace