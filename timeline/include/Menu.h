#ifndef MENU_HANDLER_H
#define MENU_HANDLER_H

#include <wx/event.h>
#include <wx/menu.h>

#include "TimeLinePart.h"

namespace gui { namespace timeline {

/**
* Class responsible for all interfacing to/from the sequence menu.
* This includes updating the menu given the state of the timeline, and
* handling events from the menu.
**/
class MenuHandler
    :   public wxEvtHandler
    ,   public TimeLinePart
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION METHODS
    //////////////////////////////////////////////////////////////////////////

    MenuHandler();
    virtual void init();
    virtual ~MenuHandler();

    //////////////////////////////////////////////////////////////////////////
    // MENU INTERFACE
    //////////////////////////////////////////////////////////////////////////

    /**
    * Only used for (re)setting the menu towards
    **/
    wxMenu* getMenu();

    void update();

    //////////////////////////////////////////////////////////////////////////
    // SEQUENCE MENU
    //////////////////////////////////////////////////////////////////////////

    void onAddVideoTrack(wxCommandEvent& WXUNUSED(event));
    void onAddAudioTrack(wxCommandEvent& WXUNUSED(event));
    void onDeleteMarked(wxCommandEvent& WXUNUSED(event));
    void onDeleteUnmarked(wxCommandEvent& WXUNUSED(event));
    void onRemoveMarkers(wxCommandEvent& WXUNUSED(event));
    void onCloseSequence(wxCommandEvent& WXUNUSED(event));

private:

    wxMenu mMenu;

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

};

}} // namespace

#endif // MENU_HANDLER_H