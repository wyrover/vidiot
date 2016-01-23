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

#include "Player.h"

#include "Config.h"
#include "Convert.h"
#include "Cursor.h"
#include "EditDisplay.h"
#include "ids.h"
#include "Sequence.h"
#include "Timeline.h"
#include "TimelinesView.h"
#include "VideoDisplay.h"
#include "VideoDisplayEvent.h"
#include "Window.h"
#include <wx/popupwin.h>

namespace gui {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

Player::Player(wxWindow *parent, model::SequencePtr sequence, wxWindow* focus)
: wxPanel(parent, wxID_ANY)
, mFocus(focus)
, mBmpHome(util::window::getIcon("icon-home.png"))
, mBmpEnd(util::window::getIcon("icon-end.png"))
, mBmpNext(util::window::getIcon("icon-next.png"))
, mBmpPrevious(util::window::getIcon("icon-previous.png"))
, mBmpPausePlay(util::window::getIcon("icon-pauseplay.png"))
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
    mStatusPanelSizer->Add(mStatus, 1, wxEXPAND);

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

    mButtonsPanelSizer->Add(mHomeButton,        wxSizerFlags(1).Expand());
    mButtonsPanelSizer->Add(mPreviousButton,    wxSizerFlags(1).Expand());
    mButtonsPanelSizer->Add(mPlayButton,        wxSizerFlags(1).Expand());
    mButtonsPanelSizer->Add(mNextButton,        wxSizerFlags(1).Expand());
    mButtonsPanelSizer->Add(mEndButton,         wxSizerFlags(1).Expand());
    mButtonsPanelSizer->Add(mSpeedButton,       wxSizerFlags(1).Expand());

    mButtonsPanel->SetSizer(mButtonsPanelSizer);

    //////////////////////////////////////////////////////////////////////////

    wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(mDisplay,        wxSizerFlags(1).Expand());
    sizer->Add(mEdit,           wxSizerFlags(1).Expand());
    sizer->Add(mStatusPanel,    wxSizerFlags(0).Expand());
    sizer->Add(mButtonsPanel,   wxSizerFlags(0).Expand());
    sizer->Hide(mEdit);
    SetSizerAndFit(sizer);

    updateLength();

    //////////////////////////////////////////////////////////////////////////
    // Speed popup window

    mSpeedSliderFrame = new wxPopupTransientWindow(this);
    mSpeedSliderFrame->SetSizer(new wxBoxSizer{ wxVERTICAL });
    mSpeedSlider = new wxSlider(mSpeedSliderFrame, wxID_ANY, VideoDisplay::sDefaultSpeed, VideoDisplay::sMinimumSpeed, VideoDisplay::sMaximumSpeed, wxDefaultPosition, wxDefaultSize, wxSL_VERTICAL | wxSL_INVERSE);
    mSpeedSlider->SetThumbLength(5);
    mSpeed100 = new wxButton(mSpeedSliderFrame, wxID_ANY, "100");
    mSpeedSliderFrame->SetBackgroundColour(mSpeedButton->GetBackgroundColour());
    mSpeedSlider->SetBackgroundColour(mSpeedButton->GetBackgroundColour());
    mSpeed100->SetBackgroundColour(mSpeedButton->GetBackgroundColour());
    mSpeedSliderFrame->GetSizer()->Add(mSpeed100, wxSizerFlags(0).Center());
    mSpeedSliderFrame->GetSizer()->Add(new wxStaticText(mSpeedSliderFrame, wxID_ANY, wxString::Format("%d", VideoDisplay::sMaximumSpeed)), wxSizerFlags(0).Center());
    mSpeedSliderFrame->GetSizer()->Add(mSpeedSlider, wxSizerFlags(1).Expand());
    mSpeedSliderFrame->GetSizer()->Add(new wxStaticText(mSpeedSliderFrame, wxID_ANY, wxString::Format("%d", VideoDisplay::sMinimumSpeed)), wxSizerFlags(0).Center());
    mSpeedSliderFrame->Fit();
    mSpeedSliderFrame->SetSize(wxSize(40, 160));
    mSpeed100->Bind(wxEVT_BUTTON, &Player::onSpeed100, this);
    mSpeedSlider->Bind(wxEVT_COMMAND_SLIDER_UPDATED, &Player::onSpeedSliderUpdate, this);
}

Player::~Player()
{
    VAR_DEBUG(this);

    Config::get().Unbind(EVENT_CONFIG_UPDATED, &Player::onConfigUpdated, this);

    mSpeed100->Unbind(wxEVT_BUTTON, &Player::onSpeed100, this);
    mSpeedSlider->Unbind(wxEVT_COMMAND_SLIDER_UPDATED, &Player::onSpeedSliderUpdate, this);
    delete mSpeedSliderFrame;

    mHomeButton->Unbind(wxEVT_COMMAND_BUTTON_CLICKED, &Player::onHome, this);
    mPreviousButton->Unbind(wxEVT_COMMAND_BUTTON_CLICKED, &Player::onPrevious, this);
    mPlayButton->Unbind(wxEVT_COMMAND_BUTTON_CLICKED, &Player::onPlay, this);
    mNextButton->Unbind(wxEVT_COMMAND_BUTTON_CLICKED, &Player::onNext, this);
    mEndButton->Unbind(wxEVT_COMMAND_BUTTON_CLICKED, &Player::onEnd, this);
    mSpeedButton->Unbind(wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, &Player::onSpeed, this);

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
    showPlayer();
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
    if (mDisplay->isPlaying())
    {
        mDisplay->moveTo(mPosition);
    }
}

void Player::moveTo(pts position)
{
    VAR_INFO(this)(position);
    mDisplay->moveTo(position);
}

void Player::showPreview(const boost::shared_ptr<wxBitmap>& bitmap)
{
    if (!GetSizer()->IsShown(mEdit))
    {
        startEdit();
    }
    mEdit->show(bitmap);
}

void Player::showPlayer()
{
    if (GetSizer()->IsShown(mEdit))
    {
        // Ensure that the proper frame is shown in the player, after editing (and previewing)
        mDisplay->moveTo(mPosition);
        endEdit();
    }
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

void Player::setSpeed(int speed)
{
    stop();
    mDisplay->setSpeed(speed);
    updateSpeedButton();
}

void Player::playRange(pts from, pts to)
{
    VAR_DEBUG(from)(to);
    stop();
    mDisplay->playRange(from, to);
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
    wxPoint pos{ mSpeedButton->GetScreenPosition() };
    wxSize buttonSize{ mSpeedButton->GetSize() };
    wxSize frameSize{ mSpeedSliderFrame->GetSize() };
    pos.x += (buttonSize.GetWidth() - frameSize.GetWidth()) / 2;
    pos.y -= frameSize.GetHeight();
    mSpeedSliderFrame->Move(pos);
    mSpeedSlider->SetValue(mDisplay->getSpeed());
    mSpeedSliderFrame->Popup();
    event.Skip();
}

void Player::onSpeed100(wxCommandEvent& event)
{
    LOG_INFO;
    setSpeed(VideoDisplay::sDefaultSpeed);
    mSpeedSliderFrame->Dismiss();
    event.Skip();
}

void Player::onSpeedSliderUpdate(wxCommandEvent& event)
{
    VAR_INFO(mSpeedSlider->GetValue());
    setSpeed(mSpeedSlider->GetValue());
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
    GetSizer()->Hide(mDisplay);
    GetSizer()->Show(mEdit);
    GetSizer()->Layout();
}

void Player::endEdit()
{
    GetSizer()->Hide(mEdit);
    GetSizer()->Show(mDisplay);
    GetSizer()->Layout();
    mEdit->show(nullptr);
}

void Player::updateStatus()
{
    milliseconds time = model::Convert::ptsToTime(mPosition);
    bool showHours =  time >= sHour;
    wxString s = model::Convert::ptsToHumanReadibleString(mPosition, true, showHours) + " / " + model::Convert::ptsToHumanReadibleString(mLength, true, showHours);
    if (Config::get().read<bool>(Config::sPathDebugShowFrameNumbers))
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
    if (!mDisplay->isPlaying() && event.getValue().IsSameAs(Config::sPathVideoShowBoundingBox))
    {
        mDisplay->moveTo(mPosition); // Update the bounding box immediately
    }
    // else: During the playback updated frames are already generated
    event.Skip();
}


//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void Player::serialize(Archive & ar, const unsigned int version)
{
    try
    {
        int speed = mDisplay->getSpeed();
        ar & boost::serialization::make_nvp("speed", speed);

        if (Archive::is_loading::value)
        {
            mDisplay->setSpeed(speed);
            updateSpeedButton();
        }
    }
    catch (boost::exception &e)                  { VAR_ERROR(boost::diagnostic_information(e)); throw; }
    catch (std::exception& e)                    { VAR_ERROR(e.what());                         throw; }
    catch (...)                                  { LOG_ERROR;                                   throw; }
}

template void Player::serialize<boost::archive::xml_oarchive>(boost::archive::xml_oarchive& ar, const unsigned int archiveVersion);
template void Player::serialize<boost::archive::xml_iarchive>(boost::archive::xml_iarchive& ar, const unsigned int archiveVersion);

} // namespace
