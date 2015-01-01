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

#ifndef MENU_HANDLER_H
#define MENU_HANDLER_H

#include "Part.h"
#include "Enums.h"

namespace gui {

    class PlaybackActiveEvent;

    namespace timeline {

/// Class responsible for all interfacing to/from the sequence menu.
/// This includes updating the menu given the state of the timeline, and
/// handling events from the menu.
class MenuHandler
    :   public wxEvtHandler // MUST BE FIRST INHERITED CLASS FOR WXWIDGETS EVENTS TO BE RECEIVED.
    ,   public Part
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION METHODS
    //////////////////////////////////////////////////////////////////////////

    MenuHandler(Timeline* timeline);
    virtual ~MenuHandler();

    void onPlaybackActive(PlaybackActiveEvent& event);

    //////////////////////////////////////////////////////////////////////////
    // NON MENU EVENTS
    //////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////
    // MENU INTERFACE
    //////////////////////////////////////////////////////////////////////////

    wxMenu* getMenu();

    void updateItems();

    void onTriggerPopupMenu(wxCommandEvent& event);
    void popup();

    void activate(bool active);

    bool isPopupShown() const;

    //////////////////////////////////////////////////////////////////////////
    // SEQUENCE MENU
    //////////////////////////////////////////////////////////////////////////

    void onAddVideoTrack(wxCommandEvent& event);
    void onAddAudioTrack(wxCommandEvent& event);
    void onSplitAtCursor(wxCommandEvent& event);
    void onRemoveEmptyTracks(wxCommandEvent& event);
    void onDeleteMarked(wxCommandEvent& event);
    void onDeleteUnmarked(wxCommandEvent& event);
    void onRemoveMarkers(wxCommandEvent& event);
    void onRemoveAllEmpty(wxCommandEvent& event);
    void onDeleteSelectedClips(wxCommandEvent& event);
    void onRenderSettings(wxCommandEvent& event);
    void onRenderSequence(wxCommandEvent& event);
    void onRenderAll(wxCommandEvent& event);
    void onCloseSequence(wxCommandEvent& event);

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    wxMenu mMenu;
    bool mActive;
    wxPoint mPopupPosition;
    bool mPopup;

};

}} // namespace

#endif
