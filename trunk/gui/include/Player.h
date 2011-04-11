#ifndef PLAYER_H
#define PLAYER_H

#include <wx/tglbtn.h>
#include <wx/button.h>
#include <wx/control.h>
#include <wx/panel.h>
#include <wx/slider.h>
#include <wx/textctrl.h>
#include <wx/minifram.h>
#include <boost/shared_ptr.hpp>
#include "UtilInt.h"

namespace model {
class Sequence;
typedef boost::shared_ptr<Sequence> SequencePtr;
}

namespace gui {

class VideoDisplay;
class EditDisplay;
class PlaybackPositionEvent;

class Player
    :   public wxPanel
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION METHODS
    //////////////////////////////////////////////////////////////////////////

    Player(wxWindow *parent, model::SequencePtr sequence);
    virtual ~Player();

    //////////////////////////////////////////////////////////////////////////
    // CONTROL METHODS
    //////////////////////////////////////////////////////////////////////////

    EditDisplay* startEdit();
    void endEdit();

    void play();
    void stop();
    void moveTo(pts position);

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

    VideoDisplay* mDisplay;
    EditDisplay* mEdit;
    wxTextCtrl* mStatus;
    int mPosition;

    //////////////////////////////////////////////////////////////////////////
    // SPEED SLIDER
    //////////////////////////////////////////////////////////////////////////

    wxButton* mHomeButton;
    wxButton* mPreviousButton;
    wxButton* mPauseButton;
    wxButton* mPlayButton;
    wxButton* mNextButton;
    wxButton* mEndButton;
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

#endif // PLAYER_H
