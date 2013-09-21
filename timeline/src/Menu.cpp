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
#include "CreateTransition.h"
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
    auto add = [&menu](int id, wxString text, bool show, bool enable, bool separate)
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
    BOOST_FOREACH( model::IClipPtr selectedClip, getSequence()->getSelectedClips() )
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

    add(ID_ADD_INTRANSITION,    _("Fade &in"),                              clickedOnMediaClip, canFadeIn,              false);
    add(ID_ADD_OUTTRANSITION,   _("Fade &out"),                             clickedOnMediaClip, canFadeOut,             false);
    add(ID_ADD_INOUTTRANSITION, _("Cross-fade from &previous"),             clickedOnMediaClip, canFadeFromPrevious,    false);
    add(ID_ADD_OUTINTRANSITION, _("Cross-fade to &next"),                   clickedOnMediaClip, canFadeToNext,          false);
    add(ID_REMOVE_EMPTY,        _("Remove &empty space"),                   clickedOnEmptyClip, clickedOnEmptyClip,     true);
    add(ID_DELETE_CLIPS,        _("&Delete selected\tDel"),                 selectedMediaClip,  !selectedEmptyClip,     true);
    add(ID_DELETE_TRIM_CLIPS,   _("Delete and &Trim selected\tShift+Del"),  selectedMediaClip,  !selectedEmptyClip,     false);

    if (menu.GetMenuItemCount() > 0)
    {
        mPopup = true;

        int result = getTimeline().GetPopupMenuSelectionFromUser(menu);
        mPopup = false;
        switch (result)
        {
        case ID_ADD_INTRANSITION:
            createTransition(model::TransitionTypeIn);
            break;
        case ID_ADD_OUTTRANSITION:
            createTransition(model::TransitionTypeOut);
            break;
        case ID_ADD_INOUTTRANSITION:
            createTransition(model::TransitionTypeInOut);
            break;
        case ID_ADD_OUTINTRANSITION:
            createTransition(model::TransitionTypeOutIn);
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

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void MenuHandler::createTransition(model::TransitionType type)
{
    VAR_INFO(type);
    PointerPositionInfo info = getMouse().getInfo(mPopupPosition);
    ASSERT(info.clip);
    model::TransitionPtr transition = info.clip->isA<model::VideoClip>() ? model::video::VideoTransitionFactory::get().getDefault() : model::audio::AudioTransitionFactory::get().getDefault();

    command::TrimClip* trimLeftCommand = 0;
    command::TrimClip* trimRightCommand = 0;
    command::CreateTransition* createTransitionCommand = new command::CreateTransition(getSequence(), info.getLogicalClip(), transition, type);

    if (createTransitionCommand->isPossible())
    {
        createTransitionCommand->submit();
        createTransitionCommand = 0;
    }
    else
    {
        pts defaultSize = Config::ReadLong(Config::sPathDefaultTransitionLength);

        if (type == model::TransitionTypeInOut || type == model::TransitionTypeOutIn)
        {
            // Ensure that the transition can be made by shortening the clips, if required (and, if possible)
            model::IClipPtr leftClip = createTransitionCommand->getLeftClip();
            ASSERT(leftClip);
            model::IClipPtr prevClip = leftClip->getPrev(); // Temporarily stored to retrieve the (new) trimmed clips again. NOTE: This may be 0 if leftClip is the first clip of the track!!!

            pts extraNeededLeft = leftClip->getMaxAdjustEnd() - (defaultSize / 2);
            if (extraNeededLeft < 0)
            {
                trimLeftCommand = new command::TrimClip(getSequence(), leftClip, model::TransitionPtr(), ClipEnd);
                trimLeftCommand->update(extraNeededLeft, true);
            }

            leftClip = prevClip ? prevClip->getNext() : info.track->getClips().front(); // Left clip is changed by the trim left command
            model::IClipPtr rightClip = leftClip->getNext();

            pts extraNeededRight = rightClip->getMinAdjustBegin() + (defaultSize / 2);
            if (extraNeededRight > 0)
            {
                trimRightCommand = new command::TrimClip(getSequence(), rightClip, model::TransitionPtr(), ClipBegin);
                trimRightCommand->update(extraNeededRight, true);
            }

            delete createTransitionCommand;
            createTransitionCommand = new command::CreateTransition(getSequence(), leftClip, transition, model::TransitionTypeOutIn);

            if (createTransitionCommand->isPossible())
            {
                ::command::Combiner* combiner = new ::command::Combiner();
                if (trimLeftCommand)
                {
                    combiner->add(trimLeftCommand);
                }
                if (trimRightCommand)
                {
                    combiner->add(trimRightCommand);
                }
                combiner->add(createTransitionCommand);
                combiner->submit();

                trimLeftCommand = 0; // Ownership transferred to Combiner
                trimRightCommand = 0; // Ownership transferred to Combiner
                createTransitionCommand = 0; // Ownership transferred to Combiner
            }
        }
        delete trimLeftCommand; // Triggers a revert also
        delete trimRightCommand; // Triggers a revert also
        if (createTransitionCommand) // If the command was not submitted, then that was because 'isPossible()' never returned true.
        {
            gui::StatusBar::get().timedInfoText(_("Unable to make room for adding the transition."));
        }
        delete createTransitionCommand;
    }
}

        //    command::TrimClip* trimLeftCommand = new command::TrimClip(getSequence(), leftClip, model::TransitionPtr(), ClipEnd);
        //    trimLeftCommand->update(-1 * (defaultSize / 2), true); // Try to use defaultTransitionSize / 2 // todo heb ik niet het dubbele nodig?
        //    if (!trimLeftCommand || trimLeftCommand->getDiff() != 0) // Any trim greater than 0 will do
        //    {
        //        leftClip = prevClip ? prevClip->getNext() : info.track->getClips().front(); // Changed by the command

        //        command::TrimClip* trimRightCommand = new command::TrimClip(getSequence(), leftClip->getNext(), model::TransitionPtr(), ClipBegin);
        //        trimRightCommand->update(defaultSize / 2, true); // Try to use defaultTransitionSize / 2// todo heb ik niet het dubbele nodig?
        //        if (!trimRightCommand || trimRightCommand->getDiff() != 0) // Any trim greater than 0 will do
        //        {
        //            command::CreateTransition* newcmd = new command::CreateTransition(getSequence(), leftClip, transition, model::TransitionTypeOutIn);

        //            if (newcmd->isPossible())
        //            {
        //                ::command::Combiner* combiner = new ::command::Combiner();
        //                combiner->add(trimLeftCommand);
        //                trimLeftCommand = 0; // Ownership transferred to Combiner
        //                combiner->add(trimRightCommand);
        //                trimRightCommand = 0; // Ownership transferred to Combiner
        //                combiner->add(newcmd);
        //                combiner->submit();
        //                showMessage = false;
        //            }
        //        }
        //    }
        //    if (trimLeftCommand)
        //    {
        //        delete trimLeftCommand; // Triggers a revert also
        //    }
        //    if (trimRightCommand)
        //    {
        //        delete trimRightCommand; // Triggers a revert also
        //    }
        //}
        //delete cmd;
        //if (showMessage)
        //{
        //    gui::StatusBar::get().timedInfoText(_("Unable to make room for adding the transition."));
        //}

}} // namespace