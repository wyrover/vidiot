#include "GuiPlayer.h"

#include <wx/bitmap.h>
#include <wx/sizer.h>
#include <wx/slider.h>
#include <wx/textctrl.h>
#include <wx/stattext.h>
#include "UtilLog.h"
#include "GuiTimeLine.h"
#include "GuiTimeLineZoom.h"
#include "GuiVideoDisplay.h"
#include "preview-home.xpm" 
#include "preview-end.xpm" 
#include "preview-next.xpm" 
#include "preview-play.xpm" 
#include "preview-pause.xpm" 
#include "preview-previous.xpm" 
#include "Constants.h"

namespace gui {

static const int sMinimumSpeed = 50;
static const int sMaximumSpeed = 200;
static const int sDefaultSpeed = 100;

wxBitmap bmpHome    (preview_home_xpm);
wxBitmap bmpEnd     (preview_end_xpm);
wxBitmap bmpNext    (preview_next_xpm);
wxBitmap bmpPlay    (preview_play_xpm);
wxBitmap bmpPrevious(preview_previous_xpm);
wxBitmap bmpPause   (preview_pause_xpm);

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

GuiPlayer::GuiPlayer(wxWindow *parent, timeline::GuiTimeLinePtr timeline)
:   wxPanel(parent, wxID_ANY)
,   mTimeLine(timeline)
,   mPosition(0)
,   mSpeedButton(0)
,   mSpeedSliderFrame(0)
,   mSpeedSlider(0)
{
	LOG_INFO;

    //////////////////////////////////////////////////////////////////////////

    mDisplay = new GuiVideoDisplay(this, timeline->getSequence());
    mDisplay->Bind(GUI_EVENT_PLAYBACK_POSITION, &GuiPlayer::OnPlaybackPosition, this);
    mDisplay->setSpeed(sDefaultSpeed);

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

    wxButton* home      = new wxButton(mButtonsPanel, wxID_ANY);
    wxButton* previous  = new wxButton(mButtonsPanel, wxID_ANY);
    wxButton* pause     = new wxButton(mButtonsPanel, wxID_ANY);
    wxButton* play      = new wxButton(mButtonsPanel, wxID_ANY);
    wxButton* next      = new wxButton(mButtonsPanel, wxID_ANY);
    wxButton* end       = new wxButton(mButtonsPanel, wxID_ANY);

    mSpeedButton        = new wxToggleButton(mButtonsPanel, wxID_ANY, "");
    updateSpeedButton();

    home    ->SetBitmap(bmpHome,        wxTOP);
    previous->SetBitmap(bmpPrevious,    wxTOP);
    pause   ->SetBitmap(bmpPause,       wxTOP);
    play    ->SetBitmap(bmpPlay,        wxTOP);
    next    ->SetBitmap(bmpNext,        wxTOP);
    end     ->SetBitmap(bmpEnd,         wxTOP);

    home        ->Bind(wxEVT_COMMAND_BUTTON_CLICKED,        &GuiPlayer::OnHome,     this);
    previous    ->Bind(wxEVT_COMMAND_BUTTON_CLICKED,        &GuiPlayer::OnPrevious, this);
    pause       ->Bind(wxEVT_COMMAND_BUTTON_CLICKED,        &GuiPlayer::OnPause,    this);
    play        ->Bind(wxEVT_COMMAND_BUTTON_CLICKED,        &GuiPlayer::OnPlay,     this);
    next        ->Bind(wxEVT_COMMAND_BUTTON_CLICKED,        &GuiPlayer::OnNext,     this);
    end         ->Bind(wxEVT_COMMAND_BUTTON_CLICKED,        &GuiPlayer::OnEnd,      this);
    mSpeedButton->Bind(wxEVT_COMMAND_TOGGLEBUTTON_CLICKED,  &GuiPlayer::OnSpeed,    this);

    mButtonsPanelSizer->Add(home,           wxSizerFlags(1).Expand().Bottom().Center());
    mButtonsPanelSizer->Add(previous,       wxSizerFlags(1).Expand().Bottom().Center());
    mButtonsPanelSizer->Add(pause,          wxSizerFlags(1).Expand().Bottom().Center());
    mButtonsPanelSizer->Add(play,           wxSizerFlags(1).Expand().Bottom().Center());
    mButtonsPanelSizer->Add(next,           wxSizerFlags(1).Expand().Bottom().Center());
    mButtonsPanelSizer->Add(end,            wxSizerFlags(1).Expand().Bottom().Center());
    mButtonsPanelSizer->Add(mSpeedButton,   wxSizerFlags(1).Expand().Bottom().Center());

    mButtonsPanel->SetSizer(mButtonsPanelSizer);

    //////////////////////////////////////////////////////////////////////////

    wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(mDisplay,        wxSizerFlags(1).Expand().Top().Center());
    sizer->Add(mStatusPanel,    wxSizerFlags(0).Expand().Bottom().Center());
    sizer->Add(mButtonsPanel,   wxSizerFlags(0).Expand().Bottom().Center());
    SetSizerAndFit(sizer);
}

GuiPlayer::~GuiPlayer()
{
    if (mDisplay != 0)
    {
        mDisplay->moveTo(0);
        delete mDisplay;
        mDisplay = 0;
    }
}

//////////////////////////////////////////////////////////////////////////
// CONTROL METHODS
//////////////////////////////////////////////////////////////////////////

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

void GuiPlayer::OnPlaybackPosition(GuiEventPlaybackPosition& event)
{
    mPosition = event.getValue();//getPts();
    int time = timeline::GuiTimeLineZoom::ptsToTime(mPosition);
    wxDateTime t(time / timeline::Constants::sHour, (time % timeline::Constants::sHour) / timeline::Constants::sMinute, (time % timeline::Constants::sMinute) / timeline::Constants::sSecond, time % timeline::Constants::sSecond);
    wxString s = t.Format("%H:%M:%S.%l") + wxString::Format(" [%10d]", mPosition);
    mStatus->ChangeValue(s);
    // NOT: event.Skip(); - Only the player handles this event. Forwards it if necessary. This event is only needed to detach from the video display thread.
    mTimeLine->moveCursorOnPlayback(mPosition);
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

    mSpeedSlider = new wxSlider(mSpeedSliderFrame, wxID_ANY, sDefaultSpeed, sMinimumSpeed, sMaximumSpeed, wxDefaultPosition, wxDefaultSize, wxSL_VERTICAL | wxSL_INVERSE);
    sizer->Add(new wxStaticText(mSpeedSliderFrame,wxID_ANY, wxString::Format("%d", sMaximumSpeed)), wxSizerFlags(0).Center());
    sizer->Add(mSpeedSlider, wxSizerFlags(1).Expand().Bottom().Center());
    sizer->Add(new wxStaticText(mSpeedSliderFrame,wxID_ANY, wxString::Format("%d", sMinimumSpeed)), wxSizerFlags(0).Center());

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
    mSpeedSlider->Unbind(wxEVT_KILL_FOCUS, &GuiPlayer::OnSpeedSliderFocusKill, this);
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
    mSpeedButton->Unbind(wxEVT_LEFT_DOWN, &GuiPlayer::OnLeftDown, this);
    Unbind(wxEVT_IDLE, &GuiPlayer::OnIdleAfterCloseSpeedSliderFrame, this);
    event.Skip();
}

void GuiPlayer::updateSpeedButton()
{
    mSpeedButton->SetLabel(wxString::Format("%d%%", mDisplay->getSpeed()));
}
} // namespace
