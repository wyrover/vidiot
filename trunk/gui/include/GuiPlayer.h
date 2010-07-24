#ifndef GUI_PLAYER_H
#define GUI_PLAYER_H

#include <wx/control.h>
#include <wx/panel.h>
#include <wx/textctrl.h>
#include <boost/cstdint.hpp>
#include <boost/shared_ptr.hpp>
#include "GuiPtr.h"

namespace gui {

class GuiVideoDisplay;
class GuiEventPlaybackPosition;

class GuiPlayer
:   public wxPanel
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION METHODS
    //////////////////////////////////////////////////////////////////////////

    GuiPlayer(wxWindow *parent, timeline::GuiTimeLinePtr timeline);
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

    timeline::GuiTimeLinePtr mTimeLine;

    GuiVideoDisplay* mDisplay;
    wxTextCtrl* mStatus;
    int mPosition;
};

} // namespace

#endif // GUI_PLAYER_H