#ifndef WINDOW_H
#define WINDOW_H

#include "IView.h"
#include "HelperPanel.h"
#include "UtilSingleInstance.h"

namespace util {
    class TestCrash;
}

namespace model {
    namespace audio {
        class AudioTransitionFactory;
    }
    class EventOpenProject;
    class EventCloseProject;
    class EventRenameProject;
    namespace video {
        class VideoTransitionFactory;
    }
}

namespace worker {
    class Worker;
}

namespace gui {

    namespace timeline {
        class Details;
    }

class Dialog;
class Layout;
class Watcher;
class Worker;
class TimelinesView;
class Preview;
class ProjectView;

typedef timeline::HelperPanel<timeline::Details> DetailsView;

class Window
    :   public wxDocParentFrame
    ,   public model::IView
    ,   public SingleInstance<Window>
{
public:

    static int sSequenceMenuIndex;

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    Window();
    void init();
    ~Window();

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
    // FILE MENU
    //////////////////////////////////////////////////////////////////////////

    void onExit(wxCommandEvent& event);

    //////////////////////////////////////////////////////////////////////////
    // VIEW MENU
    //////////////////////////////////////////////////////////////////////////

    void onSnapClips(wxCommandEvent& event);
    void onSnapCursor(wxCommandEvent& event);
    void onShowBoundingBox(wxCommandEvent& event);

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
    void onLog(wxCommandEvent& event);
    void onAbout(wxCommandEvent& event);

    //////////////////////////////////////////////////////////////////////////
    // WIDGETS HANDLING
    //////////////////////////////////////////////////////////////////////////

    TimelinesView&   getTimeLines();
    Preview&         getPreview();
    DetailsView&     getDetailsView();

    /// This must be called whenever a new (sub)widget has been added. This
    /// ensures proper layout.
    void triggerLayout();

    //////////////////////////////////////////////////////////////////////////
    // ENABLING/DISABLING MENUS
    //////////////////////////////////////////////////////////////////////////

    /// Change the sequence menu. This is used by a timeline to set the menu to
    /// the timeline's menu.
    /// \param menu 0 to indicate that the default disabled menu should be shown
    void setSequenceMenu(wxMenu* menu);

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    /// Used in tests to easily see which test is being executed
    /// \param title will be added after the original title
    void setAdditionalTitle(wxString title);

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    wxDocTemplate*      mDocTemplate;

    Dialog*             mDialog;

    Watcher*            mWatcher;
    worker::Worker*     mWorker;

    gui::Layout*        mLayout;

    Preview*		    mPreview;
    DetailsView*        mDetailsView;
    TimelinesView*      mTimelinesView;
    ProjectView*	    mProjectView;

    wxMenuBar*          menubar;        // For enabling/disabling menus
    wxMenu*             menuedit;       // For associating with do/undo
    wxMenu*             menusequence;   // For determining cleanup of the sequence menu's

    util::TestCrash*    mTestCrash;

    wxAuiManager        mUiManager;

    model::audio::AudioTransitionFactory* mAudioTransitionFactory;
    model::video::VideoTransitionFactory* mVideoTransitionFactory;

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