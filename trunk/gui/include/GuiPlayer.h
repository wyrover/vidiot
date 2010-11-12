#ifndef GUI_PLAYER_H
#define GUI_PLAYER_H

#include <wx/tglbtn.h>
#include <wx/button.h>
#include <wx/control.h>
#include <wx/panel.h>
#include <wx/slider.h>
#include <wx/textctrl.h>
#include <wx/minifram.h>
#include <boost/cstdint.hpp>
#include <boost/shared_ptr.hpp>
#include "ModelPtr.h"
#include "VideoDisplayEvent.h"

namespace gui {

class GuiVideoDisplay;

class GuiPlayer
:   public wxPanel
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION METHODS
    //////////////////////////////////////////////////////////////////////////

    GuiPlayer(wxWindow *parent, model::SequencePtr sequence);
	virtual ~GuiPlayer();

    //////////////////////////////////////////////////////////////////////////
    // CONTROL METHODS
    //////////////////////////////////////////////////////////////////////////

    void play();
    void stop();
    void moveTo(boost::int64_t position);

    //////////////////////////////////////////////////////////////////////////
    // GUI EVENTS
    //////////////////////////////////////////////////////////////////////////

    void onPlaybackPosition(PlaybackPositionEvent& event);
    void OnHome(wxCommandEvent& WXUNUSED(event));
    void OnPrevious(wxCommandEvent& WXUNUSED(event));
    void OnPause(wxCommandEvent& WXUNUSED(event));
    void OnPlay(wxCommandEvent& WXUNUSED(event));
    void OnNext(wxCommandEvent& WXUNUSED(event));
    void OnEnd(wxCommandEvent& WXUNUSED(event));
    void OnSpeed(wxCommandEvent& WXUNUSED(event));


private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    GuiVideoDisplay* mDisplay;
    wxTextCtrl* mStatus;
    int mPosition;

    //////////////////////////////////////////////////////////////////////////
    // SPEED SLIDER
    //////////////////////////////////////////////////////////////////////////

    wxToggleButton* mSpeedButton;
    wxMiniFrame* mSpeedSliderFrame;
    wxSlider* mSpeedSlider;

    void OnSpeedSliderFocusKill(wxFocusEvent& WXUNUSED(event) );
    void OnSpeedSliderUpdate( wxCommandEvent& WXUNUSED(event) );
    void OnIdleAfterCloseSpeedSliderFrame(wxIdleEvent& event);
    void OnLeftDown(wxMouseEvent& event);

    void updateSpeedButton();

};

} // namespace

#endif // GUI_PLAYER_H
