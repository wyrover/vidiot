#ifndef GUI_WINDOW_H
#define GUI_WINDOW_H

#include <wx/aui/aui.h>
#include <wx/menu.h>
#include <wx/docview.h>
#include <boost/serialization/access.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/version.hpp>
#include "Sequence.h"

class GuiTimelinesView;
class GuiPreview;
class GuiProjectView;
class ProjectEventOpenProject;
class ProjectEventCloseProject;
namespace model { class Project; }

class GuiWindow : public wxDocParentFrame
{
public:

    GuiWindow();
    void init();
    ~GuiWindow();

    //////////////////////////////////////////////////////////////////////////
    // PROJECT EVENTS
    //////////////////////////////////////////////////////////////////////////

    void OnOpenProject( ProjectEventOpenProject &event );
    void OnCloseProject( ProjectEventCloseProject &event );

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
    // SEQUENCE MENU
    //////////////////////////////////////////////////////////////////////////

    void OnPlaySequence(wxCommandEvent& WXUNUSED(event));
    void OnAddVideoTrack(wxCommandEvent& WXUNUSED(event));
    void OnAddAudioTrack(wxCommandEvent& WXUNUSED(event));
    void OnCloseSequence(wxCommandEvent& WXUNUSED(event));

    //////////////////////////////////////////////////////////////////////////
    // TOOLS MENU
    //////////////////////////////////////////////////////////////////////////

    void OnOptions(wxCommandEvent& WXUNUSED(event));

    //////////////////////////////////////////////////////////////////////////
    // HELP MENU
    //////////////////////////////////////////////////////////////////////////

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

    void EnableSequenceMenu(bool enable);

private:

    wxDocManager        mDocManager;
    wxDocTemplate*      mDocTemplate;

    GuiTimelinesView*   mTimelinesView;
    GuiPreview*		    mPreview;
    GuiProjectView*	    mProjectView;
	wxPanel*			mEditor;

    wxMenuBar*          menubar;    // For enabling/disabling menus
    wxMenu*             menuedit;   // For associating with do/undo

    wxAuiManager mUiManager;

    model::Project* mProject;
    std::list<model::SequencePtr> mOpenSequences;

    //////////////////////////////////////////////////////////////////////////
    // SERIALIZATION 
    //////////////////////////////////////////////////////////////////////////

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);
};

BOOST_CLASS_VERSION(GuiWindow, 1)

#endif // GUI_WINDOW_H