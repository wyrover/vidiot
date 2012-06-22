#ifndef MENU_HANDLER_H
#define MENU_HANDLER_H

#include <wx/event.h>
#include <wx/menu.h>
#include "Part.h"

namespace gui { namespace timeline {

/// Class responsible for all interfacing to/from the sequence menu.
/// This includes updating the menu given the state of the timeline, and
/// handling events from the menu.
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

    wxMenu* getMenu();

    void updateItems();

    void Popup(wxPoint position);

    void activate();

    //////////////////////////////////////////////////////////////////////////
    // SEQUENCE MENU
    //////////////////////////////////////////////////////////////////////////

    void onAddVideoTrack(wxCommandEvent& event);
    void onAddAudioTrack(wxCommandEvent& event);
    void onDeleteMarked(wxCommandEvent& event);
    void onDeleteUnmarked(wxCommandEvent& event);
    void onRemoveMarkers(wxCommandEvent& event);
    void onRenderSequence(wxCommandEvent& event);
    void onCloseSequence(wxCommandEvent& event);

    //////////////////////////////////////////////////////////////////////////
    // POPUP MENU
    //////////////////////////////////////////////////////////////////////////

    void onAddInTransition(wxCommandEvent& event);

private:

    wxMenu mMenu;
};

}} // namespace

#endif // MENU_HANDLER_H