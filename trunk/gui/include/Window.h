#ifndef WINDOW_H
#define WINDOW_H

#include <wx/aui/aui.h>
#include <wx/menu.h>
#include <wx/docview.h>
#include <boost/serialization/access.hpp>
#include <boost/serialization/version.hpp>
#include <boost/scoped_ptr.hpp>
#include "IView.h"

namespace util {
    class TestCrash;
}

namespace model {
class EventOpenProject;
class EventCloseProject;
class EventRenameProject;
}

namespace gui {

class Dialog;
class Watcher;
class Worker;
class TimelinesView;
class Preview;
class ProjectView;

class Window
    :   public wxDocParentFrame
    ,   public model::IView
{
public:

    static int sSequenceMenuIndex;

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

    /// Used for submitting project level events. These cannot be submitted by
    /// the project itselves since it involves creation/destruction of that project
    void ProcessModelEvent( wxEvent& event );

    /// Used for submitting project level events. These cannot be submitted by
    /// the project itselves since it involves creation/destruction of that project
    void QueueModelEvent( wxEvent* event );

    void onOpenProject( model::EventOpenProject &event );
    void onCloseProject( model::EventCloseProject &event );
    void onRenameProject( model::EventRenameProject &event );

    //////////////////////////////////////////////////////////////////////////
    // GUI EVENTS
    //////////////////////////////////////////////////////////////////////////

    void SetProcessingText(wxString text);

    //////////////////////////////////////////////////////////////////////////
    // FILE MENU
    //////////////////////////////////////////////////////////////////////////

    void onExit(wxCommandEvent& event);

    //////////////////////////////////////////////////////////////////////////
    // VIEW MENU
    //////////////////////////////////////////////////////////////////////////

    void onSnapClips(wxCommandEvent& event);
    void onSnapCursor(wxCommandEvent& event);

    //////////////////////////////////////////////////////////////////////////
    // SEQUENCE MENU - SEE THE TIMELINE IMPLEMENTATION
    //////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////
    // TOOLS MENU
    //////////////////////////////////////////////////////////////////////////

    void onOptions(wxCommandEvent& event);

    //////////////////////////////////////////////////////////////////////////
    // HELP MENU
    //////////////////////////////////////////////////////////////////////////

    void onHelp(wxCommandEvent& event);
    void onAbout(wxCommandEvent& event);

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

    wxDocTemplate*  mDocTemplate;

    Dialog*             mDialog;

    Watcher*            mWatcher;
    Worker*             mWorker;

    Preview*		    mPreview;
    TimelinesView*      mTimelinesView;
    ProjectView*	    mProjectView;

    wxMenuBar*          menubar;        // For enabling/disabling menus
    wxMenu*             menuedit;       // For associating with do/undo
    wxMenu*             menusequence;   // For determining cleanup of the sequence menu's

    util::TestCrash*    mTestCrash;

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
