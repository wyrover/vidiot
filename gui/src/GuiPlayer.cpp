#include "GuiPlayer.h"
#include <wx/bitmap.h>
#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/textctrl.h>
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
{
	LOG_INFO;

    //////////////////////////////////////////////////////////////////////////

    mDisplay = new GuiVideoDisplay(this, timeline->getSequence());
    mDisplay->Bind(GUI_EVENT_PLAYBACK_POSITION, &GuiPlayer::OnPlaybackPosition, this);

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

    home    ->SetBitmap(bmpHome,        wxTOP);
    previous->SetBitmap(bmpPrevious,    wxTOP);
    pause   ->SetBitmap(bmpPause,       wxTOP);
    play    ->SetBitmap(bmpPlay,        wxTOP);
    next    ->SetBitmap(bmpNext,        wxTOP);
    end     ->SetBitmap(bmpEnd,         wxTOP);

    home    ->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &GuiPlayer::OnHome,        this);
    previous->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &GuiPlayer::OnPrevious,    this);
    pause   ->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &GuiPlayer::OnPause,       this);
    play    ->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &GuiPlayer::OnPlay,        this);
    next    ->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &GuiPlayer::OnNext,        this);
    end     ->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &GuiPlayer::OnEnd,         this);

    mButtonsPanelSizer->Add(home,       wxSizerFlags(1).Expand().Bottom().Center());
    mButtonsPanelSizer->Add(previous,   wxSizerFlags(1).Expand().Bottom().Center());
    mButtonsPanelSizer->Add(pause,      wxSizerFlags(1).Expand().Bottom().Center());
    mButtonsPanelSizer->Add(play,       wxSizerFlags(1).Expand().Bottom().Center());
    mButtonsPanelSizer->Add(next,       wxSizerFlags(1).Expand().Bottom().Center());
    mButtonsPanelSizer->Add(end,        wxSizerFlags(1).Expand().Bottom().Center());

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

} // namespace
