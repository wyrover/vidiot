#ifndef GUI_PLAYER_H
#define GUI_PLAYER_H

#include <wx/control.h>
#include <wx/panel.h>
#include <wx/textctrl.h>
#include <boost/cstdint.hpp>
#include <boost/shared_ptr.hpp>
#include "GuiPtr.h"

class GuiVideoDisplay;
class GuiEventPlaybackPosition;

class GuiPlayer
:   public wxPanel
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION METHODS
    //////////////////////////////////////////////////////////////////////////

    GuiPlayer(wxWindow *parent, GuiTimeLinePtr timeline);
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

    void OnPlaybackPosition(GuiEventPlaybackPosition& event);
    void OnHome(wxCommandEvent& WXUNUSED(event));
    void OnPrevious(wxCommandEvent& WXUNUSED(event));
    void OnPause(wxCommandEvent& WXUNUSED(event));
    void OnPlay(wxCommandEvent& WXUNUSED(event));
    void OnNext(wxCommandEvent& WXUNUSED(event));
    void OnEnd(wxCommandEvent& WXUNUSED(event));

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    GuiTimeLinePtr mTimeLine;

    GuiVideoDisplay* mDisplay;
    wxTextCtrl* mStatus;
    int mPosition;
};

typedef boost::shared_ptr<GuiPlayer> PlayerPtr;

#endif // GUI_PLAYER_H