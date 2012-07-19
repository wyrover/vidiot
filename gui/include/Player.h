#ifndef PLAYER_H
#define PLAYER_H

#include "UtilInt.h"

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

    void play();
    void stop();
    void moveTo(pts position);

    void show(boost::shared_ptr<wxBitmap> bitmap);

    wxSize getVideoSize() const;

    //////////////////////////////////////////////////////////////////////////
    // GUI EVENTS
    //////////////////////////////////////////////////////////////////////////

    void onPlaybackPosition(PlaybackPositionEvent& event);
    void onHome(wxCommandEvent& event);
    void onPrevious(wxCommandEvent& event);
    void onPause(wxCommandEvent& event);
    void onPlay(wxCommandEvent& event);
    void onNext(wxCommandEvent& event);
    void onEnd(wxCommandEvent& event);
    void onSpeed(wxCommandEvent& event);

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

    void startEdit();
    void endEdit();

    void onSpeedSliderFocusKill(wxFocusEvent& event );
    void onSpeedSliderUpdate( wxCommandEvent& event );
    void onIdleAfterCloseSpeedSliderFrame(wxIdleEvent& event);
    void onLeftDown(wxMouseEvent& event);

    void updateSpeedButton();

};

} // namespace

#endif // PLAYER_H