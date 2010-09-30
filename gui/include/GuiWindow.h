#ifndef GUI_WINDOW_H
#define GUI_WINDOW_H

#include <wx/aui/aui.h>
#include <wx/menu.h>
#include <wx/docview.h>
#include <boost/serialization/access.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/version.hpp>
#include "Project.h"

namespace gui {

class GuiTimelinesView;
class GuiPreview;
class GuiProjectView;

class GuiWindow : public wxDocParentFrame
{
public:

    GuiWindow();
    void init();
    ~GuiWindow();

    //////////////////////////////////////////////////////////////////////////
    // PROJECT EVENTS
    //////////////////////////////////////////////////////////////////////////

    void OnOpenProject( model::EventOpenProject &event );
    void OnCloseProject( model::EventCloseProject &event );

    //////////////////////////////////////////////////////////////////////////
    // GUI EVENTS
    //////////////////////////////////////////////////////////////////////////

    void OnCloseWindow(wxCloseEvent& event);
    void SetProcessingText(wxString text);

    //////////////////////////////////////////////////////////////////////////
    // FILE MENU
    //////////////////////////////////////////////////////////////////////////

    void OnExit(wxCommandEvent& WXUNUSED(event));

    //////////////////////////////////////////////////////////////////////////
    // SEQUENCE MENU - SEE THE TIMELINE IMPLEMENTATION
    //////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////
    // TOOLS MENU
    //////////////////////////////////////////////////////////////////////////

    void OnOptions(wxCommandEvent& WXUNUSED(event));

    //////////////////////////////////////////////////////////////////////////
    // HELP MENU
    //////////////////////////////////////////////////////////////////////////

    void OnHelp(wxCommandEvent& WXUNUSED(event));
    void OnAbout(wxCommandEvent& WXUNUSED(event));

    //////////////////////////////////////////////////////////////////////////
    // GET WIDGETS
    //////////////////////////////////////////////////////////////////////////

    GuiTimelinesView&   getTimeLines();
    GuiPreview&         getPreview();
    GuiProjectView&	    getProjectView();

    //////////////////////////////////////////////////////////////////////////
    // ENABLING/DISABLING MENUS
    //////////////////////////////////////////////////////////////////////////

    /**
     * Change the sequence menu. This is used by a timeline to set the menu to
     * the timeline's menu.
     * @param menu 0 to indicate that the default disabled menu should be shown
     */
    void setSequenceMenu(wxMenu* menu);

private:

    wxDocManager        mDocManager;
    wxDocTemplate*      mDocTemplate;

    GuiTimelinesView*   mTimelinesView;
    GuiPreview*		    mPreview;
    GuiProjectView*	    mProjectView;
	wxPanel*			mEditor;

    wxMenuBar*          menubar;        // For enabling/disabling menus
    wxMenu*             menuedit;       // For associating with do/undo
    wxMenu*             menusequence;   // For determining cleanup of the sequence menu's

    wxAuiManager mUiManager;

    //////////////////////////////////////////////////////////////////////////
    // SERIALIZATION
    //////////////////////////////////////////////////////////////////////////

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);
};

} // namespace

// Workaround needed to prevent compile-time errors (mpl_assertion_in_line...) with gcc
//#include  <boost/preprocessor/slot/counter.hpp>
//#include BOOST____PP_UPDATE_COUNTER()
//#line BOOST_____PP_COUNTER
BOOST_CLASS_VERSION(gui::GuiWindow, 1)

#endif // GUI_WINDOW_H
