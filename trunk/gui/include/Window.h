#ifndef WINDOW_H
#define WINDOW_H

#include <wx/aui/aui.h>
#include <wx/menu.h>
#include <wx/docview.h>
#include <boost/serialization/access.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/version.hpp>
#include <boost/scoped_ptr.hpp>
#include "Project.h"

namespace gui {

class FSWatcher;
class Worker;
class TimelinesView;
class Preview;
class ProjectView;

class Window
    :   public wxDocParentFrame
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    Window();
    void init();
    ~Window();

    static Window& get();

    //////////////////////////////////////////////////////////////////////////
    // PROJECT EVENTS
    //////////////////////////////////////////////////////////////////////////

    void onOpenProject( model::EventOpenProject &event );
    void onCloseProject( model::EventCloseProject &event );

    //////////////////////////////////////////////////////////////////////////
    // GUI EVENTS
    //////////////////////////////////////////////////////////////////////////

    void SetProcessingText(wxString text);

    //////////////////////////////////////////////////////////////////////////
    // FILE MENU
    //////////////////////////////////////////////////////////////////////////

    void onExit(wxCommandEvent& WXUNUSED(event));

    //////////////////////////////////////////////////////////////////////////
    // SEQUENCE MENU - SEE THE TIMELINE IMPLEMENTATION
    //////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////
    // TOOLS MENU
    //////////////////////////////////////////////////////////////////////////

    void onOptions(wxCommandEvent& WXUNUSED(event));

    //////////////////////////////////////////////////////////////////////////
    // HELP MENU
    //////////////////////////////////////////////////////////////////////////

    void onHelp(wxCommandEvent& WXUNUSED(event));
    void onAbout(wxCommandEvent& WXUNUSED(event));

    //////////////////////////////////////////////////////////////////////////
    // GET WIDGETS
    //////////////////////////////////////////////////////////////////////////

    TimelinesView&   getTimeLines();
    Preview&         getPreview();

    //////////////////////////////////////////////////////////////////////////
    // ENABLING/DISABLING MENUS
    //////////////////////////////////////////////////////////////////////////

    /// Change the sequence menu. This is used by a timeline to set the menu to
    /// the timeline's menu.
    /// \param menu 0 to indicate that the default disabled menu should be shown
    void setSequenceMenu(wxMenu* menu);

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    wxDocManager*   mDocManager;
    wxDocTemplate*  mDocTemplate;

    boost::scoped_ptr<FSWatcher>    mWatcher;
    boost::scoped_ptr<Worker>       mWorker;

    TimelinesView*      mTimelinesView;
    Preview*		    mPreview;
    ProjectView*	    mProjectView;
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
BOOST_CLASS_VERSION(gui::Window, 1)

#endif // WINDOW_H
