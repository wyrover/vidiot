#include "GuiPlayer.h"

#include <wx/bitmap.h>
#include <wx/sizer.h>
#include <wx/slider.h>
#include <wx/textctrl.h>
#include <wx/stattext.h>
#include "UtilLog.h"
#include "Convert.h"
#include "GuiVideoDisplay.h"
#include "EditDisplay.h"
#include "preview-home.xpm" 
#include "preview-end.xpm" 
#include "preview-next.xpm" 
#include "preview-play.xpm" 
#include "preview-pause.xpm" 
#include "preview-previous.xpm" 
#include "Constants.h"
#include "Cursor.h"
#include "VideoDisplayEvent.h"

namespace gui {

wxBitmap bmpHome    (preview_home_xpm);
wxBitmap bmpEnd     (preview_end_xpm);
wxBitmap bmpNext    (preview_next_xpm);
wxBitmap bmpPlay    (preview_play_xpm);
wxBitmap bmpPrevious(preview_previous_xpm);
wxBitmap bmpPause   (preview_pause_xpm);

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

GuiPlayer::GuiPlayer(wxWindow *parent, model::SequencePtr sequence)
:   wxPanel(parent, wxID_ANY)
,   mPosition(0)
,   mHomeButton(0)
,   mPreviousButton(0)
,   mPauseButton(0)
,   mPlayButton(0)
,   mNextButton(0)
,   mEndButton(0)
,   mSpeedButton(0)
,   mSpeedSliderFrame(0)
,   mSpeedSlider(0)
{
	VAR_DEBUG(this);

    //////////////////////////////////////////////////////////////////////////

    mDisplay = new GuiVideoDisplay(this, sequence);
    mDisplay->Bind(EVENT_PLAYBACK_POSITION, &GuiPlayer::onPlaybackPosition, this);
    mDisplay->setSpeed(GuiVideoDisplay::sDefaultSpeed);

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

    mHomeButton     = new wxButton(mButtonsPanel, wxID_ANY);
    mPreviousButton = new wxButton(mButtonsPanel, wxID_ANY);
    mPauseButton    = new wxButton(mButtonsPanel, wxID_ANY);
    mPlayButton     = new wxButton(mButtonsPanel, wxID_ANY);
    mNextButton     = new wxButton(mButtonsPanel, wxID_ANY);
    mEndButton      = new wxButton(mButtonsPanel, wxID_ANY);
    mSpeedButton    = new wxToggleButton(mButtonsPanel, wxID_ANY, "");
    updateSpeedButton();

    mHomeButton    ->SetBitmap(bmpHome,        wxTOP);
    mPreviousButton->SetBitmap(bmpPrevious,    wxTOP);
    mPauseButton   ->SetBitmap(bmpPause,       wxTOP);
    mPlayButton    ->SetBitmap(bmpPlay,        wxTOP);
    mNextButton    ->SetBitmap(bmpNext,        wxTOP);
    mEndButton     ->SetBitmap(bmpEnd,         wxTOP);

    mHomeButton     ->Bind(wxEVT_COMMAND_BUTTON_CLICKED,        &GuiPlayer::OnHome,     this);
    mPreviousButton ->Bind(wxEVT_COMMAND_BUTTON_CLICKED,        &GuiPlayer::OnPrevious, this);
    mPauseButton    ->Bind(wxEVT_COMMAND_BUTTON_CLICKED,        &GuiPlayer::OnPause,    this);
    mPlayButton     ->Bind(wxEVT_COMMAND_BUTTON_CLICKED,        &GuiPlayer::OnPlay,     this);
    mNextButton     ->Bind(wxEVT_COMMAND_BUTTON_CLICKED,        &GuiPlayer::OnNext,     this);
    mEndButton      ->Bind(wxEVT_COMMAND_BUTTON_CLICKED,        &GuiPlayer::OnEnd,      this);
    mSpeedButton    ->Bind(wxEVT_COMMAND_TOGGLEBUTTON_CLICKED,  &GuiPlayer::OnSpeed,    this);

    mButtonsPanelSizer->Add(mHomeButton,        wxSizerFlags(1).Expand().Bottom().Center());
    mButtonsPanelSizer->Add(mPreviousButton,    wxSizerFlags(1).Expand().Bottom().Center());
    mButtonsPanelSizer->Add(mPauseButton,       wxSizerFlags(1).Expand().Bottom().Center());
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
}

GuiPlayer::~GuiPlayer()
{
    VAR_DEBUG(this);

    mHomeButton     ->Unbind(wxEVT_COMMAND_BUTTON_CLICKED,        &GuiPlayer::OnHome,     this);
    mPreviousButton ->Unbind(wxEVT_COMMAND_BUTTON_CLICKED,        &GuiPlayer::OnPrevious, this);
    mPauseButton    ->Unbind(wxEVT_COMMAND_BUTTON_CLICKED,        &GuiPlayer::OnPause,    this);
    mPlayButton     ->Unbind(wxEVT_COMMAND_BUTTON_CLICKED,        &GuiPlayer::OnPlay,     this);
    mNextButton     ->Unbind(wxEVT_COMMAND_BUTTON_CLICKED,        &GuiPlayer::OnNext,     this);
    mEndButton      ->Unbind(wxEVT_COMMAND_BUTTON_CLICKED,        &GuiPlayer::OnEnd,      this);
    mSpeedButton    ->Unbind(wxEVT_COMMAND_TOGGLEBUTTON_CLICKED,  &GuiPlayer::OnSpeed,    this);

    if (mDisplay != 0)
    {
        mDisplay->Unbind(EVENT_PLAYBACK_POSITION, &GuiPlayer::onPlaybackPosition, this);
 //       mDisplay->moveTo(0);
        delete mDisplay;
        mDisplay = 0;
    }
}

//////////////////////////////////////////////////////////////////////////
// CONTROL METHODS
//////////////////////////////////////////////////////////////////////////

EditDisplay* GuiPlayer::startEdit()
{
    GetSizer()->Hide(mDisplay);
    GetSizer()->Show(mEdit);
    GetSizer()->Layout();
    return mEdit;
}

void GuiPlayer::endEdit()
{
    GetSizer()->Hide(mEdit);
    GetSizer()->Show(mDisplay);
    GetSizer()->Layout();
    mEdit->show(boost::shared_ptr<wxBitmap>());
}

void GuiPlayer::play()
{
    LOG_INFO;
    mDisplay->play();
}

void GuiPlayer::stop()
{
    LOG_INFO;
    mDisplay->moveTo(mPosition);
}

void GuiPlayer::moveTo(int64_t position)
{
    VAR_INFO(this)(position);
    mDisplay->moveTo(position);
}

//////////////////////////////////////////////////////////////////////////
// GUI EVENTS
//////////////////////////////////////////////////////////////////////////

void GuiPlayer::onPlaybackPosition(PlaybackPositionEvent& event)
{
    mPosition = event.getValue();//getPts();
    int time = model::Convert::ptsToTime(mPosition);
    wxDateTime t(time / model::Constants::sHour, (time % model::Constants::sHour) / model::Constants::sMinute, (time % model::Constants::sMinute) / model::Constants::sSecond, time % model::Constants::sSecond);
    wxString s = t.Format("%H:%M:%S.%l") + wxString::Format(" [%10d]", mPosition);
    mStatus->ChangeValue(s);

    // NOT: event.Skip(); - Only the player handles this event. Forwards it if necessary. This event is only needed to detach from the video display thread.
    GetEventHandler()->QueueEvent(new PlaybackPositionEvent(event)); // Event must be sent by the player. Other components don't see the videodisplay.
}

void GuiPlayer::OnHome(wxCommandEvent& WXUNUSED(event))
{
    LOG_INFO;
    mDisplay->moveTo(0);
}

void GuiPlayer::OnPrevious(wxCommandEvent& WXUNUSED(event))
{
    LOG_INFO;
}

void GuiPlayer::OnPause(wxCommandEvent& WXUNUSED(event))
{
    LOG_INFO;
    stop();
}

void GuiPlayer::OnPlay(wxCommandEvent& WXUNUSED(event))
{
    LOG_INFO;
    play();
}

void GuiPlayer::OnNext(wxCommandEvent& WXUNUSED(event))
{
    LOG_INFO;
}

void GuiPlayer::OnEnd(wxCommandEvent& WXUNUSED(event))
{
    LOG_INFO;
}

void GuiPlayer::OnSpeed(wxCommandEvent& WXUNUSED(event))
{
    LOG_INFO;
    /** @todo If button clicked very fast twice, the button is not depressed. */

    mSpeedSliderFrame = new wxMiniFrame(this, wxID_ANY, "title", wxDefaultPosition, wxDefaultSize, 0);

    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

    mSpeedSlider = new wxSlider(mSpeedSliderFrame, wxID_ANY, GuiVideoDisplay::sDefaultSpeed, GuiVideoDisplay::sMinimumSpeed, GuiVideoDisplay::sMaximumSpeed, wxDefaultPosition, wxDefaultSize, wxSL_VERTICAL | wxSL_INVERSE);
    sizer->Add(new wxStaticText(mSpeedSliderFrame,wxID_ANY, wxString::Format("%d", GuiVideoDisplay::sMaximumSpeed)), wxSizerFlags(0).Center());
    sizer->Add(mSpeedSlider, wxSizerFlags(1).Expand().Bottom().Center());
    sizer->Add(new wxStaticText(mSpeedSliderFrame,wxID_ANY, wxString::Format("%d", GuiVideoDisplay::sMinimumSpeed)), wxSizerFlags(0).Center());

    mSpeedSliderFrame->SetSizerAndFit(sizer);

    wxPoint pos = mSpeedButton->GetScreenPosition();
    wxSize buttonSize = mSpeedButton->GetSize();
    wxSize frameSize = mSpeedSliderFrame->GetSize();

    pos.x += (buttonSize.GetWidth() - frameSize.GetWidth())/2;
    pos.y -= frameSize.GetHeight();

    mSpeedSliderFrame->SetBackgroundColour(mSpeedButton->GetBackgroundColour());//wxColour(128,128,0));
    mSpeedSlider->SetBackgroundColour(mSpeedButton->GetBackgroundColour());

    mSpeedSlider->SetThumbLength(5);

    mSpeedSliderFrame->Move(pos);
    mSpeedSliderFrame->Show();

    mSpeedSlider->SetFocus();
    mSpeedSlider->Bind(wxEVT_KILL_FOCUS,                &GuiPlayer::OnSpeedSliderFocusKill,     this);
    mSpeedSlider->Bind(wxEVT_COMMAND_SLIDER_UPDATED,    &GuiPlayer::OnSpeedSliderUpdate,        this);
    mSpeedButton->Bind(wxEVT_LEFT_DOWN,                 &GuiPlayer::OnLeftDown,                 this);
}

void GuiPlayer::OnSpeedSliderUpdate( wxCommandEvent& WXUNUSED(event) )
{
    VAR_INFO(mSpeedSlider->GetValue());
    mDisplay->setSpeed(mSpeedSlider->GetValue());
    updateSpeedButton();
}

void GuiPlayer::OnSpeedSliderFocusKill(wxFocusEvent& event)
{
    mSpeedSliderFrame->Hide();
    mSpeedSlider->Unbind(wxEVT_KILL_FOCUS,                &GuiPlayer::OnSpeedSliderFocusKill,     this);
    mSpeedSlider->Unbind(wxEVT_COMMAND_SLIDER_UPDATED,    &GuiPlayer::OnSpeedSliderUpdate,        this);
    mSpeedButton->Unbind(wxEVT_LEFT_DOWN,                 &GuiPlayer::OnLeftDown,                 this);
    delete mSpeedSliderFrame;
    mSpeedSliderFrame = 0;
    Bind(wxEVT_IDLE, &GuiPlayer::OnIdleAfterCloseSpeedSliderFrame, this);
}

void GuiPlayer::OnLeftDown(wxMouseEvent& event)
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

void GuiPlayer::OnIdleAfterCloseSpeedSliderFrame(wxIdleEvent& event)
{
    mSpeedButton->SetValue(false);
    Unbind(wxEVT_IDLE, &GuiPlayer::OnIdleAfterCloseSpeedSliderFrame, this);
    event.Skip();
}

void GuiPlayer::updateSpeedButton()
{
    mSpeedButton->SetLabel(wxString::Format("%d%%", mDisplay->getSpeed()));
}

} // namespace
