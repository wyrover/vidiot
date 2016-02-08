// Copyright 2013-2016 Eric Raijmakers.
//
// This file is part of Vidiot.
//
// Vidiot is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Vidiot is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Vidiot. If not, see <http://www.gnu.org/licenses/>.

#pragma once

#include "IView.h"
#include "HelperPanel.h"
#include "UtilSingleInstance.h"

class RunInMainScheduler;

namespace util {
    class TestCrash;
    namespace thread {
        struct RunInMainScheduler;
    }
}

namespace model {
    class FileWatcher;
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
    class VisibleWorker;
    class InvisibleWorker;
}

namespace gui {

    namespace timeline {
        class Details;
    }

class Dialog;
class Worker;
class TimelinesView;
class Preview;
class ProjectView;
class Help;

typedef timeline::HelperPanel<timeline::Details> DetailsView;

class Window
    :   public wxDocParentFrame
    ,   public model::IView
    ,   public SingleInstance<Window>
{
public:

    static int sSequenceMenuIndex;
    static const wxString sPaneNameHelp;
    static const wxString sPaneNameProject;
    static const wxString sPaneNameDetails;
    static const wxString sPaneNamePreview;
    static const wxString sPaneNameTimelines;
    static const std::map<int, wxString> sMapMenuIdToPaneName;

    // Strings: not static/global (must initialize proper translation during startup first)
    wxString sPaneCaptionHelp;
    wxString sPaneCaptionProject;
    wxString sPaneCaptionDetails;
    wxString sPaneCaptionPreview;
    wxString sPaneCaptionTimelines;

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    Window();
    void init();
    virtual ~Window();

    //////////////////////////////////////////////////////////////////////////
    // PROJECT EVENTS
    //////////////////////////////////////////////////////////////////////////

    /// Used for submitting project level events. These cannot be submitted by
    /// the project itselves since it involves creation/destruction of that project
    void ProcessModelEvent(wxEvent& event);

    /// Used for submitting project level events. These cannot be submitted by
    /// the project itselves since it involves creation/destruction of that project
    void QueueModelEvent(wxEvent* event);

    void onOpenProject(model::EventOpenProject &event);
    void onCloseProject(model::EventCloseProject &event);
    void onRenameProject(model::EventRenameProject &event);

    //////////////////////////////////////////////////////////////////////////
    // GUI EVENTS
    //////////////////////////////////////////////////////////////////////////

    void onMove(wxMoveEvent& event);
    void onSize(wxSizeEvent& event);
    void onMaximize(wxMaximizeEvent& event);
    void onClose(wxCloseEvent& event);

    void onClosePane(wxAuiManagerEvent& event);
    void onRestorePane(wxAuiManagerEvent& event);

    //////////////////////////////////////////////////////////////////////////
    // FILE MENU
    //////////////////////////////////////////////////////////////////////////

    void onProperties(wxCommandEvent& event);
    void onNewFiles(wxCommandEvent& event);
    void onNewAutoFolder(wxCommandEvent& event);
    void onNewSequence(wxCommandEvent& event);
    void onExit(wxCommandEvent& event);

    //////////////////////////////////////////////////////////////////////////
    // VIEW MENU
    //////////////////////////////////////////////////////////////////////////

    void onSnapClips(wxCommandEvent& event);
    void onSnapCursor(wxCommandEvent& event);
    void onShowBoundingBox(wxCommandEvent& event);
    void onShowProject(wxCommandEvent& event);
    void onShowDetails(wxCommandEvent& event);
    void onShowPreview(wxCommandEvent& event);
    void onShowTimelines(wxCommandEvent& event);

    //////////////////////////////////////////////////////////////////////////
    // SEQUENCE MENU - SEE THE TIMELINE IMPLEMENTATION
    //////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////
    // TOOLS MENU
    //////////////////////////////////////////////////////////////////////////

    void onOptions(wxCommandEvent& event);

    //////////////////////////////////////////////////////////////////////////
    // WORKSPACE MENU
    //////////////////////////////////////////////////////////////////////////

    void onShowCaptions(wxCommandEvent& event);
    void onWorkspaceSave(wxCommandEvent& event);
    void onWorkspaceLoad(wxCommandEvent& event);
    void onWorkspaceDelete(wxCommandEvent& event);
    void onWorkspaceDeleteAll(wxCommandEvent& event);
    void onWorkspaceDefault(wxCommandEvent& event);
    void onWorkspaceFullscreen(wxCommandEvent& event);

    //////////////////////////////////////////////////////////////////////////
    // HELP MENU
    //////////////////////////////////////////////////////////////////////////

    void onHelp(wxCommandEvent& event);
    void onLog(wxCommandEvent& event);
    void onConfig(wxCommandEvent& event);
    void onAbout(wxCommandEvent& event);

    //////////////////////////////////////////////////////////////////////////
    // DROP FILES EVENTS
    //////////////////////////////////////////////////////////////////////////

    void onDropFiles(wxDropFilesEvent& event);

    //////////////////////////////////////////////////////////////////////////
    // WIDGETS HANDLING
    //////////////////////////////////////////////////////////////////////////

    TimelinesView&   getTimeLines();
    Preview&         getPreview();
    DetailsView&     getDetailsView();

    wxAuiManager&    getUiManager();

    /// This must be called whenever a new (sub)widget has been added. This
    /// ensures proper layout.
    void triggerLayout();

    //////////////////////////////////////////////////////////////////////////
    // ENABLING/DISABLING MENUS
    //////////////////////////////////////////////////////////////////////////

    /// Change the sequence menu. This is used by a timeline to set the menu to
    /// the timeline's menu.
    /// \param menu 0 to indicate that there is not open sequence (menu is disabled)
    /// \param enabled if menu != 0, indicates if the menu is enabled or not.
    void setSequenceMenu(wxMenu* menu, bool enabled);

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    /// Used in tests to easily see which test is being executed
    /// \param title will be added after the original title
    void setAdditionalTitle(const wxString& title);

    bool isDialogOpen() const;
    void setDialogOpen(bool open);

    bool isProjectOpened() const;

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    wxDocManager* mDocManager = nullptr;
    wxDocTemplate* mDocTemplate = nullptr;

    Dialog* mDialog = nullptr;

    model::FileWatcher* mWatcher = nullptr;

    util::thread::RunInMainScheduler* mScheduler = nullptr;

    worker::VisibleWorker*   mVisibleWorker = nullptr;
    worker::InvisibleWorker* mInvisibleWorker = nullptr;

    Preview*        mPreview = nullptr;
    DetailsView*    mDetailsView = nullptr;
    TimelinesView*  mTimelinesView = nullptr;
    ProjectView*    mProjectView = nullptr;
    Help*           mHelp = nullptr;

    wxMenuBar*          mMenuBar = nullptr;        // For enabling/disabling menus
    wxMenu*             mMenuFile = nullptr;       // For enabling/disabling menus
    wxMenu*             mMenuEdit = nullptr;       // For associating with do/undo
    wxMenu*             mMenuView = nullptr;       // For updating view items when panes are closed.
    wxMenu*             mMenuSequence = nullptr;   // For determining cleanup of the sequence menu's
    wxMenu*             mMenuWorkspace = nullptr;
    wxMenu*             mMenuHelp = nullptr;       // For updating Help when the pane is closed.
    util::TestCrash*    mTestCrash = nullptr;

    wxAuiManager        mUiManager = nullptr;

    model::audio::AudioTransitionFactory* mAudioTransitionFactory = nullptr;
    model::video::VideoTransitionFactory* mVideoTransitionFactory = nullptr;

    wxString            mDefaultPerspective;

    wxString mTitle;
    bool mProjectOpen;
    std::atomic<bool> mDialogOpen;

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    void updateViewMenu();
    void updateWorkspaceMenu();
    friend struct ViewHelper;
    void updateTitle();
    void togglePane(const wxString& title);

    //////////////////////////////////////////////////////////////////////////
    // SERIALIZATION
    //////////////////////////////////////////////////////////////////////////

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);
};

} // namespace

BOOST_CLASS_VERSION(gui::Window, 1)
