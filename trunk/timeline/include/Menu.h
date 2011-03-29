#ifndef MENU_HANDLER_H
#define MENU_HANDLER_H

#include <wx/bookctrl.h>
#include <wx/event.h>
#include <wx/menu.h>
#include "Part.h"

namespace gui { namespace timeline {

/**
* Class responsible for all interfacing to/from the sequence menu.
* This includes updating the menu given the state of the timeline, and
* handling events from the menu.
**/
class MenuHandler
    :   public wxEvtHandler // MUST BE FIRST INHERITED CLASS FOR WXWIDGETS EVENTS TO BE RECEIVED.
    ,   public Part
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION METHODS
    //////////////////////////////////////////////////////////////////////////

    MenuHandler(Timeline* timeline);
    virtual ~MenuHandler();

    //////////////////////////////////////////////////////////////////////////
    // MENU INTERFACE
    //////////////////////////////////////////////////////////////////////////

    /**
    * Only used for (re)setting the menu towards
    **/
    wxMenu* getMenu();

    void updateItems();

    void Popup(wxPoint position);

    //////////////////////////////////////////////////////////////////////////
    // SEQUENCE MENU
    //////////////////////////////////////////////////////////////////////////

    void onAddVideoTrack(wxCommandEvent& WXUNUSED(event));
    void onAddAudioTrack(wxCommandEvent& WXUNUSED(event));
    void onDeleteMarked(wxCommandEvent& WXUNUSED(event));
    void onDeleteUnmarked(wxCommandEvent& WXUNUSED(event));
    void onRemoveMarkers(wxCommandEvent& WXUNUSED(event));
    void onCloseSequence(wxCommandEvent& WXUNUSED(event));

    //////////////////////////////////////////////////////////////////////////
    // POPUP MENU
    //////////////////////////////////////////////////////////////////////////

    void onAddTransition(wxCommandEvent& WXUNUSED(event));

private:

    wxMenu mMenu;

    //////////////////////////////////////////////////////////////////////////
    // EVENTS
    //////////////////////////////////////////////////////////////////////////

    void onPageChanged(wxBookCtrlEvent& event);

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

};

}} // namespace

#endif // MENU_HANDLER_H
