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
#include "TimelineClipboard.h"
#include "TimelinesView.h"
#include "Track.h"
#include "Transition.h"
#include "TrimClip.h"
#include "UnlinkClips.h"
#include "UtilBind.h"
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
    mMenu.Append(ID_SPLIT_AT_CURSOR,   _("Split at cursor"), _("Split clips at the current cursor position."));
    mMenu.AppendSeparator();
    mMenu.Append(ID_DELETEMARKED,   _("Delete marked regions"), _("Delete all marked regions from sequence."));
    mMenu.Append(ID_DELETEUNMARKED, _("Delete unmarked regions"), _("Delete all unmarked regions from sequence."));
    mMenu.Append(ID_REMOVEMARKERS,  _("Remove markers"), _("Remove all markers from the sequence."));
    mMenu.AppendSeparator();
    mMenu.Append(ID_DELETEEMPTY,  _("Remove empty"), _("Remove all empty areas from the sequence."));
    mMenu.AppendSeparator();
    mMenu.Append(ID_RENDERSETTINGS, _("Render settings"), ("Open the dialog containing the settings for generating a movie file from the sequence."));
    mMenu.Append(ID_RENDERSEQUENCE, _("Render '") + getSequence()->getName() + "'", _("Generate movie file from sequence."));
    mMenu.Append(ID_RENDERSEQUENCE, _("Render all modified sequences"), _("Generate movie files for all sequences in the project."));
    mMenu.AppendSeparator();
    mMenu.Append(ID_CLOSESEQUENCE,  _("Close"), _("Close the sequence. Will not remove sequence from project."));

    BindAndCatchExceptions(Window::get(), wxEVT_COMMAND_MENU_SELECTED,    &MenuHandler::onAddVideoTrack,         this, ID_ADDVIDEOTRACK);
    BindAndCatchExceptions(Window::get(), wxEVT_COMMAND_MENU_SELECTED,    &MenuHandler::onAddAudioTrack,         this, ID_ADDAUDIOTRACK);
    BindAndCatchExceptions(Window::get(), wxEVT_COMMAND_MENU_SELECTED,    &MenuHandler::onRemoveEmptyTracks,     this, ID_REMOVE_EMPTY_TRACKS);

    BindAndCatchExceptions(Window::get(), wxEVT_COMMAND_MENU_SELECTED,    &MenuHandler::onSplitAtCursor,         this, ID_SPLIT_AT_CURSOR);

    BindAndCatchExceptions(Window::get(), wxEVT_COMMAND_MENU_SELECTED,    &MenuHandler::onDeleteMarked,          this, ID_DELETEMARKED);
    BindAndCatchExceptions(Window::get(), wxEVT_COMMAND_MENU_SELECTED,    &MenuHandler::onDeleteUnmarked,        this, ID_DELETEUNMARKED);
    BindAndCatchExceptions(Window::get(), wxEVT_COMMAND_MENU_SELECTED,    &MenuHandler::onRemoveMarkers,         this, ID_REMOVEMARKERS);

    BindAndCatchExceptions(Window::get(), wxEVT_COMMAND_MENU_SELECTED,    &MenuHandler::onRemoveAllEmpty,        this, ID_DELETEEMPTY);

    BindAndCatchExceptions(Window::get(), wxEVT_COMMAND_MENU_SELECTED,    &MenuHandler::onRenderSettings,        this, ID_RENDERSETTINGS);
    BindAndCatchExceptions(Window::get(), wxEVT_COMMAND_MENU_SELECTED,    &MenuHandler::onRenderSequence,        this, ID_RENDERSEQUENCE);
    BindAndCatchExceptions(Window::get(), wxEVT_COMMAND_MENU_SELECTED,    &MenuHandler::onRenderAll,             this, ID_RENDERALL);

    BindAndCatchExceptions(Window::get(), wxEVT_COMMAND_MENU_SELECTED,    &MenuHandler::onCloseSequence,         this, ID_CLOSESEQUENCE);

    BindAndCatchExceptions(getTimeline(), wxEVT_COMMAND_MENU_SELECTED,    &MenuHandler::onTriggerPopupMenu,      this, ID_TRIGGER_POPUP_MENU);

    BindAndCatchExceptions(getPlayer(), EVENT_PLAYBACK_ACTIVE,           &MenuHandler::onPlaybackActive,        this);

    updateItems();

    Window::get().setSequenceMenu(getMenu(), true);
}

MenuHandler::~MenuHandler()
{
    VAR_DEBUG(this);

    Window::get().setSequenceMenu(0, false); // If this is NOT the last timeline to be closed, then an 'activate()' will reset the menu to that other timeline

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

void MenuHandler::popup()
{
    // This is decoupled via an event, to ensure that this method returns immediately.
    // Otherwise, the state chart handling is blocked for a while (duration of popup) causing crashes in boost statechart.
    getTimeline().GetEventHandler()->QueueEvent(new wxCommandEvent(wxEVT_COMMAND_MENU_SELECTED,ID_TRIGGER_POPUP_MENU));
}

void MenuHandler::onTriggerPopupMenu(wxCommandEvent& event)
{
    ASSERT(mActive);
    LOG_INFO;

    mPopupPosition = getMouse().getRightUpPosition();
    PointerPositionInfo info = getMouse().getInfo(mPopupPosition);

    // Due to the event decoupling (see MenuHandler::popup), checks beforehand must be redone here.
    // The event ID_TRIGGER_POPUP_MENU may be triggered, but a mouse move/zoom change/scroll change
    // may be done inbetween. Hence, any 'safety checks' must be done here.

    // Mechanism:
    // Default menu options are hidden and enabled.
    // If an item is selected for which a menu option makes sense, then the option is shown.
    // If an item is selected for which a menu option does not make sense, then the option is disabled.

    wxMenu menu;

    auto add = [](wxMenu& menu, int id, wxString text, bool show, bool enable, bool separate)
    {
        if (show)
        {
            if (separate)
            {
                menu.AppendSeparator();
            }
            menu.Append(id, text);
            menu.Enable(id, enable);
        }
    };

    bool selectedMediaClip = false;
    std::set< model::IClipPtr > selection = getSequence()->getSelectedClips();
    for (model::IClipPtr selectedClip : selection)
    {
        if (selectedClip->isA<model::VideoClip>()) { selectedMediaClip = true; }
        if (selectedClip->isA<model::AudioClip>()) { selectedMediaClip = true; }
    }

    model::IClipPtr clickedClip = info.getLogicalClip();
    bool clickedOnVideoClip = (clickedClip && clickedClip->isA<model::VideoClip>());
    bool clickedOnAudioClip = (clickedClip && clickedClip->isA<model::AudioClip>());
    bool clickedOnEmptyClip = (clickedClip && clickedClip->isA<model::EmptyClip>());
    bool clickedOnMediaClip = clickedOnVideoClip || clickedOnAudioClip;

    bool canPaste = getClipboard().canPaste();

    std::map<model::TransitionType, bool> isSupported
    {
        { model::TransitionTypeFadeIn, true },
        { model::TransitionTypeFadeOut, true },
        { model::TransitionTypeFadeInFromPrevious, true },
        { model::TransitionTypeFadeOutToNext, true }
    };
    std::map<model::TransitionType, wxString> transitionMenuEntry
    {
        { model::TransitionTypeFadeIn, _("Fade in (more)") },
        { model::TransitionTypeFadeOut, _("Fade out (more)") },
        { model::TransitionTypeFadeInFromPrevious, _("Fade from previous (more)") },
        { model::TransitionTypeFadeOutToNext, _("Fade to next (more)") }
    };

    std::map<model::TransitionType, wxString> transitionMenuDescription
    {
        { model::TransitionTypeFadeIn, _("Show all fade in transitions") },
        { model::TransitionTypeFadeOut, _("Show all fade out transitions") },
        { model::TransitionTypeFadeInFromPrevious, _("Show all fade from previous transitions") },
        { model::TransitionTypeFadeOutToNext, _("Show all fade to next transitions") }
    };

    if (clickedClip)
    {
        // Check if there's already a transition, disabling the transition menu options
        model::TransitionPtr prevTransition = boost::dynamic_pointer_cast<model::Transition>(clickedClip->getPrev());
        if (prevTransition && prevTransition->getRight())
        {
            isSupported[model::TransitionTypeFadeIn] = false;
            isSupported[model::TransitionTypeFadeInFromPrevious] = false;
        }
        model::TransitionPtr nextTransition = boost::dynamic_pointer_cast<model::Transition>(clickedClip->getNext());
        if (nextTransition && nextTransition->getLeft())
        {
            isSupported[model::TransitionTypeFadeOut] = false;
            isSupported[model::TransitionTypeFadeOutToNext] = false;
        }

        // Check if the clicked clip has room for making a crossfade
        if (clickedClip->getMaxAdjustBegin() <= 0)
        {
            isSupported[model::TransitionTypeFadeInFromPrevious] = false;
        }
        if (clickedClip->getMinAdjustEnd() >= 0)
        {
            isSupported[model::TransitionTypeFadeOutToNext] = false;
        }

        // Check if the adjacent clip has room for making a crossfade
        if (!clickedClip->getPrev() ||
            clickedClip->getPrev()->isA<model::EmptyClip>() ||
            clickedClip->getPrev()->getMinAdjustEnd() >= 0)
        {
            isSupported[model::TransitionTypeFadeInFromPrevious] = false;
        }
        if (!clickedClip->getNext() ||
            clickedClip->getNext()->isA<model::EmptyClip>() ||
            clickedClip->getNext()->getMaxAdjustBegin() <= 0)
        {
            isSupported[model::TransitionTypeFadeOutToNext] = false;
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
            ASSERT_EQUALS(clip1, clip2->getLink());
            unlink.push_back(clip1);
            unlink.push_back(clip2);
            enableUnlink = true;
        }
    }

    if (clickedOnAudioClip || clickedOnVideoClip)
    {
        add(menu, ID_ADD_INTRANSITION, _("Fade &in"), clickedOnMediaClip, isSupported[model::TransitionTypeFadeIn], false);
        add(menu, ID_ADD_OUTTRANSITION, _("Fade &out"), clickedOnMediaClip, isSupported[model::TransitionTypeFadeOut], false);
        add(menu, ID_ADD_INOUTTRANSITION, _("Cross-fade from &previous"), clickedOnMediaClip, isSupported[model::TransitionTypeFadeInFromPrevious], false);
        add(menu, ID_ADD_OUTINTRANSITION, _("Cross-fade to &next"), clickedOnMediaClip, isSupported[model::TransitionTypeFadeOutToNext], false);
    }
    std::map<int, model::TransitionType> mapMenuItemToTransitionType;
    std::map<int, model::TransitionPtr> mapMenuItemToTransition;
    if (clickedOnVideoClip) // For audio clips there is only the crossfade
    {
        std::map<model::TransitionType, wxMenu*> transitionMenus
        {
            { model::TransitionTypeFadeIn, new wxMenu },  // On heap, destroyed when toplevel menu destroyed
            { model::TransitionTypeFadeOut, new wxMenu },
            { model::TransitionTypeFadeInFromPrevious, new wxMenu },
            { model::TransitionTypeFadeOutToNext, new wxMenu }
        };

        int id = ID_POPUP_END;

        menu.AppendSeparator();
        for (auto type_and_menu : transitionMenus)
        {
            model::TransitionType type{type_and_menu.first};
            for (model::TransitionPtr transition : model::video::VideoTransitionFactory::get().getAllPossibleTransitions())
            {
                if (transition->supports(type))
                {
                    add(*type_and_menu.second, id, transition->getDescription(type), clickedOnMediaClip, isSupported[type], false);
                    mapMenuItemToTransitionType[id] = type;
                    mapMenuItemToTransition[id] = transition;
                    id++;
                }
            }
            int id{ menu.AppendSubMenu(type_and_menu.second,transitionMenuEntry[type],transitionMenuDescription[type])->GetId() };
            menu.Enable(id, isSupported[type]);
        }
    }

    add(menu, wxID_CUT, _("Cut"), true, selectedMediaClip, true);
    add(menu, wxID_COPY, _("Copy"), true, selectedMediaClip, false);
    add(menu, wxID_PASTE, _("Paste here"), true, canPaste, false);
    add(menu, ID_REMOVE_EMPTY, _("Remove &empty space"), clickedOnEmptyClip, clickedOnEmptyClip, true);
    add(menu, ID_DELETE_CLIPS, _("&Delete selected\tDel"), selectedMediaClip, true, true);
    add(menu, ID_DELETE_TRIM_CLIPS, _("Delete and &Trim selected\tShift+Del"), selectedMediaClip, true, false);
    add(menu, ID_UNLINK_CLIPS, _("&Unlink audio and video clips"), selectedMediaClip, enableUnlink, true);

    if (menu.GetMenuItemCount() > 0)
    {
        bool enabledEntry = false;
        for (wxMenuItem* item : menu.GetMenuItems())
        {
            if (!item->IsSeparator() && 
                item->IsEnabled())
            {
                enabledEntry = true;
                break;
            }
        }
        if (!enabledEntry)
        {
            // Do not show popup menu if there are no enabled entries.
            return;
        }

        mPopup = true;

        model::TransitionPtr defaultTransition = clickedOnVideoClip ? model::video::VideoTransitionFactory::get().getDefault() : model::audio::AudioTransitionFactory::get().getDefault();

        int result = getTimeline().GetPopupMenuSelectionFromUser(menu);
        mPopup = false;
        switch (result)
        {
        case wxID_NONE:
            break;
        case ID_ADD_INTRANSITION:
            command::createTransition(getSequence(), info.getLogicalClip(), model::TransitionTypeFadeIn, defaultTransition);
            break;
        case ID_ADD_OUTTRANSITION:
            command::createTransition(getSequence(), info.getLogicalClip(), model::TransitionTypeFadeOut, defaultTransition);
            break;
        case ID_ADD_INOUTTRANSITION:
            command::createTransition(getSequence(), info.getLogicalClip(), model::TransitionTypeFadeInFromPrevious, defaultTransition);
            break;
        case ID_ADD_OUTINTRANSITION:
            command::createTransition(getSequence(), info.getLogicalClip(), model::TransitionTypeFadeOutToNext, defaultTransition);
            break;
        case wxID_CUT:
            getClipboard().onCut();
            break;
        case wxID_COPY:
            getClipboard().onCopy();
            break;
        case wxID_PASTE:
            getClipboard().onPaste(false);
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
            (new command::UnlinkClips(getSequence(), unlink))->submit();
            break;
        default:
            if (clickedOnVideoClip && result >= ID_POPUP_END)
            {
                // Selected one of the video transitions
                ASSERT_MAP_CONTAINS(mapMenuItemToTransitionType, result);
                ASSERT_MAP_CONTAINS(mapMenuItemToTransition, result);
                command::createTransition(getSequence(), info.getLogicalClip(), mapMenuItemToTransitionType[result], mapMenuItemToTransition[result]);
            }
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
    // NOT: event.Skip():
    // In some tests (particularly, TestSavingAndLoading::testLoadOldVersions)
    // there were crashes where this event could not be handled anymore by
    // the gui::Window class. What happened:
    // - event triggered by menu option
    // - dialog shown modal
    // - test cases sees 'main window lost focus'
    // - test case hits 'escape' and triggers 'close document' almost immediately
    // - document is closed (including close timeline and thus the Menu class)
    // - the ShowModal returns and the event cannot be handled anymore -> crash
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
    // NOT: event.Skip() -- see MenuHandler::onRenderSettings for rationale
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
