// Copyright 2013-2016 Eric Raijmakers.
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

#pragma once

#include "IPlayer.h"

class EventConfigUpdated;
class wxPopupTransientWindow;

namespace gui {

class EditDisplay;
class PlaybackActiveEvent;
class PlaybackPositionEvent;
class VideoDisplay;

class Player
    : public wxPanel
    , public IPlayer
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
    void moveTo(pts position);

    ResumeInfo pause() override;
    void resume(const ResumeInfo& info) override;

    /// Show a preview of an edit operation
    /// \param bitmap image to be shown
    void showPreview(const wxBitmapPtr& bitmap);

    /// Show the video player (and not the preview)
    void showPlayer();

    wxSize getVideoSize() const;

    void updateLength();

    void setSpeed(int speed);

    void playRange(pts from, pts to);
    void stopRange();

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

    wxWindow* mFocus = nullptr;
    VideoDisplay* mDisplay = nullptr;
    EditDisplay* mEdit = nullptr;
    wxTextCtrl* mStatus = nullptr;
    pts mLength = 0; ///< Length of sequence
    pts mPosition = 0;

    //////////////////////////////////////////////////////////////////////////
    // BUTTONS & BITMAPS
    //////////////////////////////////////////////////////////////////////////

    wxBitmap mBmpHome;
    wxBitmap mBmpEnd;
    wxBitmap mBmpNext;
    wxBitmap mBmpPrevious;
    wxBitmap mBmpPausePlay;

    wxButton* mHomeButton = nullptr;
    wxButton* mPreviousButton = nullptr;
    wxButton* mPlayButton = nullptr;
    wxButton* mNextButton = nullptr;
    wxButton* mEndButton = nullptr;
    wxToggleButton* mSpeedButton = nullptr;

    //////////////////////////////////////////////////////////////////////////
    // SPEED SLIDER
    //////////////////////////////////////////////////////////////////////////


    wxPopupTransientWindow* mSpeedSliderFrame = nullptr;
    wxButton* mSpeed100 = nullptr;
    wxSlider* mSpeedSlider = nullptr;

    void onSpeed100(wxCommandEvent& event);
    void onSpeedSliderUpdate( wxCommandEvent& event);

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
