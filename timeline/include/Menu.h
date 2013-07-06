#ifndef MENU_HANDLER_H
#define MENU_HANDLER_H

#include "Part.h"
#include "Enums.h"

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

    void onTriggerPopupMenu(wxCommandEvent& event);
    void Popup(wxPoint position);

    void activate(bool active);

    bool isPopupShown() const;

    //////////////////////////////////////////////////////////////////////////
    // SEQUENCE MENU
    //////////////////////////////////////////////////////////////////////////

    void onAddVideoTrack(wxCommandEvent& event);
    void onAddAudioTrack(wxCommandEvent& event);
    void onSplitAtCursor(wxCommandEvent& event);
    void onRemoveEmptyTracks(wxCommandEvent& event);
    void onDeleteMarked(wxCommandEvent& event);
    void onDeleteUnmarked(wxCommandEvent& event);
    void onRemoveMarkers(wxCommandEvent& event);
    void onRemoveAllEmpty(wxCommandEvent& event);
    void onRenderSettings(wxCommandEvent& event);
    void onRenderSequence(wxCommandEvent& event);
    void onRenderAll(wxCommandEvent& event);
    void onCloseSequence(wxCommandEvent& event);

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    wxMenu mMenu;
    bool mActive;
    wxPoint mPopupPosition;
    bool mPopup;

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    void createTransition(model::TransitionType type);

};

}} // namespace

#endif // MENU_HANDLER_H