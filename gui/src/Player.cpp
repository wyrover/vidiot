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

#include "Player.h"

#include "Config.h"
#include "Constants.h"
#include "Convert.h"
#include "Cursor.h"
#include "EditDisplay.h"
#include "ids.h"
#include "Sequence.h"
#include "Timeline.h"
#include "TimelinesView.h"
#include "UtilLog.h"
#include "VideoDisplay.h"
#include "VideoDisplayEvent.h"
#include "Window.h"
#include <wx/minifram.h>

namespace gui {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

Player::Player(wxWindow *parent, model::SequencePtr sequence, wxWindow* focus)
: wxPanel(parent, wxID_ANY)
, mFocus(focus)
, mLength(0)
, mPosition(0)
, mHomeButton(0)
, mPreviousButton(0)
, mPlayButton(0)
, mNextButton(0)
, mEndButton(0)
, mSpeedButton(0)
, mSpeedSliderFrame(0)
, mSpeedSlider(0)
, mBmpHome(util::window::getIcon("preview-home.png"))
, mBmpEnd(util::window::getIcon("preview-end.png"))
, mBmpNext(util::window::getIcon("preview-next.png"))
, mBmpPrevious(util::window::getIcon("preview-previous.png"))
, mBmpPausePlay(util::window::getIcon("preview-pauseplay.png"))
{
    VAR_DEBUG(this);

    //////////////////////////////////////////////////////////////////////////

    Config::get().Bind(EVENT_CONFIG_UPDATED, &Player::onConfigUpdated, this);

    //////////////////////////////////////////////////////////////////////////

    mDisplay = new VideoDisplay(this, sequence);
    mDisplay->Bind(EVENT_PLAYBACK_ACTIVE, &Player::onPlaybackActive, this);
    mDisplay->Bind(EVENT_PLAYBACK_POSITION, &Player::onPlaybackPosition, this);
    mDisplay->setSpeed(VideoDisplay::sDefaultSpeed);

    //////////////////////////////////////////////////////////////////////////

    mEdit = new EditDisplay(this);

    //////////////////////////////////////////////////////////////////////////

    wxPanel* mStatusPanel = new wxPanel(this);
    wxBoxSizer* mStatusPanelSizer = new wxBoxSizer(wxHORIZONTAL);

    mStatus = new wxTextCtrl(mStatusPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY | wxTE_CENTRE);
    wxTextAttr textattr(*wxWHITE,*wxBLACK,*wxNORMAL_FONT,wxTEXT_ALIGNMENT_CENTER);
    mStatus->SetDefaultStyle(textattr);
    mStatusPanelSizer->Add(mStatus, 1, wxEXPAND | wxALIGN_CENTER);

    mStatusPanel->SetSizer(mStatusPanelSizer);

    //////////////////////////////////////////////////////////////////////////

    wxPanel* mButtonsPanel = new wxPanel(this);
    wxBoxSizer* mButtonsPanelSizer = new wxBoxSizer(wxHORIZONTAL);

    mHomeButton     = new wxButton(mButtonsPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
    mPreviousButton = new wxButton(mButtonsPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
    mPlayButton     = new wxButton(mButtonsPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
    mNextButton     = new wxButton(mButtonsPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
    mEndButton      = new wxButton(mButtonsPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
    mSpeedButton    = new wxToggleButton(mButtonsPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
    updateSpeedButton();

    mHomeButton    ->SetBitmap(mBmpHome,        wxTOP);
    mPreviousButton->SetBitmap(mBmpPrevious,    wxTOP);
    mPlayButton    ->SetBitmap(mBmpPausePlay,   wxTOP);
    mNextButton    ->SetBitmap(mBmpNext,        wxTOP);
    mEndButton     ->SetBitmap(mBmpEnd,         wxTOP);

    mHomeButton     ->Bind(wxEVT_COMMAND_BUTTON_CLICKED,        &Player::onHome,     this);
    mPreviousButton ->Bind(wxEVT_COMMAND_BUTTON_CLICKED,        &Player::onPrevious, this);
    mPlayButton     ->Bind(wxEVT_COMMAND_BUTTON_CLICKED,        &Player::onPlay,     this);
    mNextButton     ->Bind(wxEVT_COMMAND_BUTTON_CLICKED,        &Player::onNext,     this);
    mEndButton      ->Bind(wxEVT_COMMAND_BUTTON_CLICKED,        &Player::onEnd,      this);
    mSpeedButton    ->Bind(wxEVT_COMMAND_TOGGLEBUTTON_CLICKED,  &Player::onSpeed,    this);

    mButtonsPanelSizer->Add(mHomeButton,        wxSizerFlags(1).Expand().Bottom().Center());
    mButtonsPanelSizer->Add(mPreviousButton,    wxSizerFlags(1).Expand().Bottom().Center());
    mButtonsPanelSizer->Add(mPlayButton,        wxSizerFlags(1).Expand().Bottom().Center());
    mButtonsPanelSizer->Add(mNextButton,        wxSizerFlags(1).Expand().Bottom().Center());
    mButtonsPanelSizer->Add(mEndButton,         wxSizerFlags(1).Expand().Bottom().Center());
    mButtonsPanelSizer->Add(mSpeedButton,       wxSizerFlags(1).Expand().Bottom().Center());

    mButtonsPanel->SetSizer(mButtonsPanelSizer);

    //////////////////////////////////////////////////////////////////////////

    wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(mDisplay,        wxSizerFlags(1).Expand().Top().Center());
    sizer->Add(mEdit,           wxSizerFlags(1).Expand().Top().Center());
    sizer->Add(mStatusPanel,    wxSizerFlags(0).Expand().Bottom().Center());
    sizer->Add(mButtonsPanel,   wxSizerFlags(0).Expand().Bottom().Center());
    sizer->Hide(mEdit);
    SetSizerAndFit(sizer);

    updateLength();
}

Player::~Player()
{
    VAR_DEBUG(this);

    Config::get().Unbind(EVENT_CONFIG_UPDATED, &Player::onConfigUpdated, this);

    mHomeButton     ->Unbind(wxEVT_COMMAND_BUTTON_CLICKED,        &Player::onHome,     this);
    mPreviousButton ->Unbind(wxEVT_COMMAND_BUTTON_CLICKED,        &Player::onPrevious, this);
    mPlayButton     ->Unbind(wxEVT_COMMAND_BUTTON_CLICKED,        &Player::onPlay,     this);
    mNextButton     ->Unbind(wxEVT_COMMAND_BUTTON_CLICKED,        &Player::onNext,     this);
    mEndButton      ->Unbind(wxEVT_COMMAND_BUTTON_CLICKED,        &Player::onEnd,      this);
    mSpeedButton    ->Unbind(wxEVT_COMMAND_TOGGLEBUTTON_CLICKED,  &Player::onSpeed,    this);

    ASSERT(mDisplay);
    ASSERT(mEdit);
    mDisplay->Unbind(EVENT_PLAYBACK_ACTIVE, &Player::onPlaybackActive, this);
    mDisplay->Unbind(EVENT_PLAYBACK_POSITION, &Player::onPlaybackPosition, this);
}

//////////////////////////////////////////////////////////////////////////
// CONTROL METHODS
//////////////////////////////////////////////////////////////////////////

void Player::play()
{
    LOG_INFO;
    ASSERT(wxThread::IsMain());
    if (GetSizer()->IsShown(mEdit))
    {
        endEdit();
    }
    mDisplay->play();
}

void Player::play_pause()
{
    if (mDisplay->isPlaying())
    {
        stop();
    }
    else
    {
        play();
    }
}

void Player::stop()
{
    LOG_INFO;
    ASSERT(wxThread::IsMain());
    mDisplay->moveTo(mPosition);
}

void Player::moveTo(pts position)
{
    VAR_INFO(this)(position);
    if (GetSizer()->IsShown(mEdit))
    {
        endEdit();
    }
    mDisplay->moveTo(position);
}

void Player::show(const boost::shared_ptr<wxBitmap>& bitmap)
{
    if (!GetSizer()->IsShown(mEdit))
    {
        startEdit();
    }
    mEdit->show(bitmap);
}

wxSize Player::getVideoSize() const
{
    if (GetSizer()->IsShown(mEdit))
    {
        return mEdit->GetClientSize();
    }
    return mDisplay->GetClientSize();
}

void Player::updateLength()
{
    mLength = mDisplay->getSequence()->getLength();
    updateStatus();
}

//////////////////////////////////////////////////////////////////////////
// GUI EVENTS
//////////////////////////////////////////////////////////////////////////

void Player::onPlaybackActive(PlaybackActiveEvent& event)
{
    // NOT: event.Skip(); - Only the player handles this event.
    GetEventHandler()->QueueEvent(new PlaybackActiveEvent(event)); // Event must be sent by the player. Other components don't see the videodisplay.
}

void Player::onPlaybackPosition(PlaybackPositionEvent& event) // make playbackstart/stop event for letting the timeline know
{
    mPosition = event.getValue();
    updateStatus();

    // NOT: event.Skip(); - Only the player handles this event. Forwards it if necessary. This event is only needed to detach from the video display thread.
    GetEventHandler()->QueueEvent(new PlaybackPositionEvent(event)); // Event must be sent by the player. Other components don't see the videodisplay.

    if (mDisplay->isPlaying() && (mPosition > mDisplay->getSequence()->getLength()))
    {
        stop(); // Stop at end of sequence
    }
}

void Player::onHome(wxCommandEvent& event)
{
    LOG_INFO;
    TimelinesView::get().getTimeline(mDisplay->getSequence()).getCursor().home();
    mFocus->SetFocus();
}

void Player::onPrevious(wxCommandEvent& event)
{
    LOG_INFO;
    TimelinesView::get().getTimeline(mDisplay->getSequence()).getCursor().prevCut();
    mFocus->SetFocus();
}

void Player::onPlay(wxCommandEvent& event)
{
    LOG_INFO;
    play_pause();
    mFocus->SetFocus();
}

void Player::onNext(wxCommandEvent& event)
{
    LOG_INFO;
    TimelinesView::get().getTimeline(mDisplay->getSequence()).getCursor().nextCut();
    mFocus->SetFocus();
}

void Player::onEnd(wxCommandEvent& event)
{
    LOG_INFO;
    TimelinesView::get().getTimeline(mDisplay->getSequence()).getCursor().end();
    mFocus->SetFocus();
}

void Player::onSpeed(wxCommandEvent& event)
{
    LOG_INFO;

    mSpeedSliderFrame = new wxMiniFrame(this, wxID_ANY, "title", wxDefaultPosition, wxDefaultSize, 0);

    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

    mSpeedSlider = new wxSlider(mSpeedSliderFrame, wxID_ANY, VideoDisplay::sDefaultSpeed, VideoDisplay::sMinimumSpeed, VideoDisplay::sMaximumSpeed, wxDefaultPosition, wxDefaultSize, wxSL_VERTICAL | wxSL_INVERSE);
    sizer->Add(new wxStaticText(mSpeedSliderFrame,wxID_ANY, wxString::Format("%d", VideoDisplay::sMaximumSpeed)), wxSizerFlags(0).Center());
    sizer->Add(mSpeedSlider, wxSizerFlags(1).Expand().Bottom().Center());
    sizer->Add(new wxStaticText(mSpeedSliderFrame,wxID_ANY, wxString::Format("%d", VideoDisplay::sMinimumSpeed)), wxSizerFlags(0).Center());

    mSpeedSliderFrame->SetSizerAndFit(sizer);

    wxPoint pos = mSpeedButton->GetScreenPosition();
    wxSize buttonSize = mSpeedButton->GetSize();
    wxSize frameSize = mSpeedSliderFrame->GetSize();

    pos.x += (buttonSize.GetWidth() - frameSize.GetWidth())/2;
    pos.y -= frameSize.GetHeight();

    mSpeedSliderFrame->SetBackgroundColour(mSpeedButton->GetBackgroundColour());
    mSpeedSlider->SetBackgroundColour(mSpeedButton->GetBackgroundColour());

    mSpeedSlider->SetThumbLength(5);

    mSpeedSliderFrame->Move(pos);
    mSpeedSliderFrame->Show();

    mSpeedSlider->SetFocus();
    mSpeedSlider->Bind(wxEVT_KILL_FOCUS,                &Player::onSpeedSliderFocusKill,     this);
    mSpeedSlider->Bind(wxEVT_COMMAND_SLIDER_UPDATED,    &Player::onSpeedSliderUpdate,        this);
    mSpeedButton->Bind(wxEVT_LEFT_DOWN,                 &Player::onLeftDown,                 this);
}


void Player::onSpeedSliderUpdate( wxCommandEvent& event )
{
    VAR_INFO(mSpeedSlider->GetValue());
    mDisplay->setSpeed(mSpeedSlider->GetValue());
    updateSpeedButton();
}

void Player::onSpeedSliderFocusKill(wxFocusEvent& event)
{
    mSpeedSliderFrame->Hide();
    mSpeedSlider->Unbind(wxEVT_KILL_FOCUS,                &Player::onSpeedSliderFocusKill,     this);
    mSpeedSlider->Unbind(wxEVT_COMMAND_SLIDER_UPDATED,    &Player::onSpeedSliderUpdate,        this);
    mSpeedButton->Unbind(wxEVT_LEFT_DOWN,                 &Player::onLeftDown,                 this);
    delete mSpeedSliderFrame;
    mSpeedSliderFrame = 0;
    Bind(wxEVT_IDLE, &Player::onIdleAfterCloseSpeedSliderFrame, this);
}

void Player::onLeftDown(wxMouseEvent& event)
{
    // NOT: event.Skip();
    // By not calling Skip, the event handling for the toggle button is blocked
    // here. The speed frame can now be closed/ended by two methods:
    // 1. Click outside speed frame
    // 2. Press toggle button again.
    // All attempts to handle this via 'outoffocus' for case 1, and the regular
    // toggle button event ('OnSpeed' again) didn't work. This was caused by
    // first handling the 'outoffocus' event when the button is pressed
    // (then first the speed frame goes out of focus) and subsequently, another
    // press event was generated, which would be done on the now depressed
    // button that had been closed in handling 'outoffocus'.
    //
    // Now, all exit handling is done via the 'outoffocus' method, and the
    // extra button press is suppressed here. Since the button must be
    // enabled again, the Idle event handling was introduced.
}

void Player::onIdleAfterCloseSpeedSliderFrame(wxIdleEvent& event)
{
    mSpeedButton->SetValue(false);
    Unbind(wxEVT_IDLE, &Player::onIdleAfterCloseSpeedSliderFrame, this);
    mFocus->SetFocus();
    event.Skip();
}

void Player::updateSpeedButton()
{
    mSpeedButton->SetLabel(wxString::Format("%d%%", mDisplay->getSpeed()));
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void Player::startEdit()
{
    // todo GCC: Very first trim operation the edit feedback is a gray bg only (until first screen refresh?)
    GetSizer()->Hide(mDisplay);
    GetSizer()->Show(mEdit);
    GetSizer()->Layout();
}

void Player::endEdit()
{
    GetSizer()->Hide(mEdit);
    GetSizer()->Show(mDisplay);
    GetSizer()->Layout();
    mEdit->show(boost::shared_ptr<wxBitmap>());
}

void Player::updateStatus()
{
    milliseconds time = model::Convert::ptsToTime(mPosition);
    bool showHours =  time >= model::Constants::sHour;
    wxDateTime t(time / model::Constants::sHour, (time % model::Constants::sHour) / model::Constants::sMinute, (time % model::Constants::sMinute) / model::Constants::sSecond, time % model::Constants::sSecond);
    wxString s = model::Convert::ptsToHumanReadibleString(mPosition, true, showHours) + " / " + model::Convert::ptsToHumanReadibleString(mLength, true, showHours);
    if (Config::ReadBool(Config::sPathDebugShowFrameNumbers))
    {
        s += wxString::Format(" [%10" PRId64 "]", mPosition);
    }
    mStatus->ChangeValue(s);
}

//////////////////////////////////////////////////////////////////////////
// CONFIG UDPATES
//////////////////////////////////////////////////////////////////////////

void Player::onConfigUpdated(EventConfigUpdated& event)
{
    if (!mDisplay->isPlaying() && event.getValue().IsSameAs(Config::sPathShowBoundingBox))
    {
        mDisplay->moveTo(mPosition); // Update the bounding box immediately
    }
    // else: During the playback updated frames are already generated
    event.Skip();
}

} // namespace
