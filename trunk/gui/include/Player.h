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

#ifndef PLAYER_H
#define PLAYER_H

#include "UtilInt.h"

class EventConfigUpdated;
class wxMiniFrame;

namespace gui {

class EditDisplay;
class PlaybackActiveEvent;
class PlaybackPositionEvent;
class VideoDisplay;

class Player
    :   public wxPanel
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION METHODS
    //////////////////////////////////////////////////////////////////////////

    Player(wxWindow *parent, model::SequencePtr sequence, wxWindow* focus);
    virtual ~Player();

    //////////////////////////////////////////////////////////////////////////
    // CONTROL METHODS
    //////////////////////////////////////////////////////////////////////////

    void play();
    void play_pause();
    void stop();
    void moveTo(pts position);

    /// Show a preview of an edit operation
    /// \param bitmap image to be shown
    void showPreview(const wxBitmapPtr& bitmap);

    /// Show the video player (and not the preview)
    void showPlayer();

    wxSize getVideoSize() const;

    void updateLength();

    //////////////////////////////////////////////////////////////////////////
    // GUI EVENTS
    //////////////////////////////////////////////////////////////////////////

    void onPlaybackActive(PlaybackActiveEvent& event);
    void onPlaybackPosition(PlaybackPositionEvent& event);
    void onHome(wxCommandEvent& event);
    void onPrevious(wxCommandEvent& event);
    void onPlay(wxCommandEvent& event);
    void onNext(wxCommandEvent& event);
    void onEnd(wxCommandEvent& event);
    void onSpeed(wxCommandEvent& event);

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    wxWindow* mFocus;
    VideoDisplay* mDisplay;
    EditDisplay* mEdit;
    wxTextCtrl* mStatus;
    pts mLength; ///< Length of sequence
    pts mPosition;

    //////////////////////////////////////////////////////////////////////////
    // BUTTONS & BITMAPS
    //////////////////////////////////////////////////////////////////////////

    wxBitmap mBmpHome;
    wxBitmap mBmpEnd;
    wxBitmap mBmpNext;
    wxBitmap mBmpPrevious;
    wxBitmap mBmpPausePlay;

    wxButton* mHomeButton;
    wxButton* mPreviousButton;
    wxButton* mPlayButton;
    wxButton* mNextButton;
    wxButton* mEndButton;
    wxToggleButton* mSpeedButton;

    //////////////////////////////////////////////////////////////////////////
    // SPEED SLIDER
    //////////////////////////////////////////////////////////////////////////


    wxMiniFrame* mSpeedSliderFrame;
    wxSlider* mSpeedSlider;

    void onSpeedSliderFocusKill(wxFocusEvent& event);
    void onSpeedSliderUpdate( wxCommandEvent& event);
    void onIdleAfterCloseSpeedSliderFrame(wxIdleEvent& event);
    void onLeftDown(wxMouseEvent& event);

    void updateSpeedButton();

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    void startEdit();
    void endEdit();
    void updateStatus();

    //////////////////////////////////////////////////////////////////////////
    // CONFIG UDPATES
    //////////////////////////////////////////////////////////////////////////

    void onConfigUpdated(EventConfigUpdated& event);

    //////////////////////////////////////////////////////////////////////////
    // SERIALIZATION
    //////////////////////////////////////////////////////////////////////////

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);
};

} // namespace

#endif
