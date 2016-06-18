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

#include "Window.h"

#include "AudioTransitionFactory.h"
#include "CommandLine.h"
#include "CommandProcessor.h"
#include "Config.h"
#include "Dialog.h"
#include "DialogAbout.h"
#include "DialogNewProject.h"
#include "DialogOptions.h"
#include "DialogProjectProperties.h"
#include "FileAnalyzer.h"
#include "Help.h"
#include "ids.h"
#include "Node.h"
#include "Preview.h"
#include "Project.h"
#include "ProjectEvent.h"
#include "ProjectModification.h"
#include "ProjectView.h"
#include "ProjectViewAddAsset.h"
#include "StatusBar.h"
#include "TimelinesView.h"
#include "UtilTestCrash.h"
#include "UtilThread.h"
#include "UtilWindow.h"
#include "VideoTransitionFactory.h"
#include "FileWatcher.h"
#include "Worker.h"

namespace gui {

const wxString Window::sPaneNameHelp("Help");
const wxString Window::sPaneNameProject("Project");
const wxString Window::sPaneNameDetails("Details");
const wxString Window::sPaneNamePreview("Preview");
const wxString Window::sPaneNameTimelines("Timelines");

const std::map<int, wxString> Window::sMapMenuIdToPaneName = {
    { static_cast<int>(ID_SHOW_PROJECT), Window::sPaneNameProject },
    { static_cast<int>(ID_SHOW_DETAILS), Window::sPaneNameDetails },
    { static_cast<int>(ID_SHOW_PREVIEW), Window::sPaneNamePreview },
    { static_cast<int>(ID_SHOW_TIMELINES), Window::sPaneNameTimelines },
    { static_cast<int>(wxID_HELP), Window::sPaneNameHelp }
};

//////////////////////////////////////////////////////////////////////////
// HELPER CLASSES
//////////////////////////////////////////////////////////////////////////

struct ViewHelper : public wxView
{
    ViewHelper() : wxView() {};
    virtual ~ViewHelper() {};
    void OnDraw(wxDC *dc) override {}
    void OnUpdate(wxView *sender, wxObject *hint = NULL) override {}
    bool OnCreate(wxDocument *doc, long flags) override
    {
        Activate(true); // Make sure the document manager knows that this is the get view.
        return true;
    }
    bool OnClose(bool deleteWindow = true) override
    {
        Activate(false);
        return true;
    }
    DECLARE_DYNAMIC_CLASS(ViewHelper);
};
IMPLEMENT_DYNAMIC_CLASS(ViewHelper, wxView);

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

const int sStatusProcessing = 8;
int Window::sSequenceMenuIndex = 0;
#ifndef NDEBUG
static const wxString sTitleDebug(" (DEBUG)");
#else
static const wxString sTitleDebug("");
#endif

Window::Window()
    : wxDocParentFrame()
    , sPaneCaptionHelp(_("Help"))
    , sPaneCaptionProject(_("Project"))
    , sPaneCaptionDetails(_("Details"))
    , sPaneCaptionPreview(_("Preview"))
    , sPaneCaptionTimelines(_("Timelines"))
    , mDocManager(new wxDocManager())
    // TRANSLATORS: %s == Name of application (executable)
    , mDocTemplate(new wxDocTemplate(mDocManager, wxString::Format(_("%s files"), CommandLine::get().ExeName), "*." + model::Project::sFileExtension, "", model::Project::sFileExtension, wxString::Format(_("%s Project"), CommandLine::get().ExeName), wxString::Format(_("%s Project View"), CommandLine::get().ExeName), CLASSINFO(model::Project), CLASSINFO(ViewHelper)))
    , mDialog(new Dialog())
    , mAudioTransitionFactory(new model::audio::AudioTransitionFactory())
    , mVideoTransitionFactory(new model::video::VideoTransitionFactory())
    , mTitle{ CommandLine::get().ExeName + sTitleDebug }
    , mDialogOpen{ false }
{
    Create(mDocManager, 0, wxID_ANY, mTitle, wxDefaultPosition, wxSize(1000,700));

    DragAcceptFiles(true);

    // Construction not done in constructor list due to dependency on sCurrent
    mVisibleWorker   = new worker::VisibleWorker();
    mInvisibleWorker = new worker::InvisibleWorker();
    mPreview         = new Preview(this); // Must be opened before timelinesview for the case of autoloading with open sequences/timelines
    mDetailsView     = new DetailsView(this);
    mTimelinesView   = new TimelinesView(this);
    mProjectView     = new ProjectView(this);
    mHelp            = new Help(this);
    mScheduler = new util::thread::RunInMainScheduler();

    util::window::setIcons(this);

    mMenuFile = new wxMenu();
    mMenuFile->Append(wxID_NEW);
    mMenuFile->Append(wxID_OPEN);
    mMenuFile->Append(wxID_CLOSE);

    mMenuFile->Append(wxID_REVERT, _("Revert"));
    mMenuFile->Append(wxID_SAVE);
    mMenuFile->Append(wxID_SAVEAS);
    mMenuFile->AppendSeparator();
    mMenuFile->Append(ID_NEW_FILES, _("Add files"), _("Add media files from disk."));
    mMenuFile->Append(ID_NEW_AUTOFOLDER, _("Add folder"), _("Add folder with media files from disk."));
    mMenuFile->Append(ID_NEW_SEQUENCE, _("Add sequence"), _("Add new (blank) movie sequence."));
    mMenuFile->AppendSeparator();
    mMenuFile->Append(wxID_PROPERTIES);
    mMenuFile->AppendSeparator();
    mMenuFile->Append(wxID_EXIT, _("Exit"));
    mMenuFile->Enable(ID_NEW_FILES,false);
    mMenuFile->Enable(ID_NEW_AUTOFOLDER,false);
    mMenuFile->Enable(ID_NEW_SEQUENCE,false);
    mMenuFile->Enable(wxID_PROPERTIES,false);

    mMenuEdit = new wxMenu();
    mMenuEdit->Append(wxID_UNDO);
    mMenuEdit->Append(wxID_REDO);
    mMenuEdit->AppendSeparator();
    mMenuEdit->Append(wxID_CUT);
    mMenuEdit->Append(wxID_COPY);
    mMenuEdit->Append(wxID_PASTE);
	mMenuEdit->Enable(wxID_CUT,false);
	mMenuEdit->Enable(wxID_COPY,false);
	mMenuEdit->Enable(wxID_PASTE,false);

    mMenuView = new wxMenu();
    mMenuView->AppendCheckItem(ID_SHOW_PROJECT, sPaneCaptionProject);
    mMenuView->AppendCheckItem(ID_SHOW_DETAILS, sPaneCaptionDetails);
    mMenuView->AppendCheckItem(ID_SHOW_PREVIEW, sPaneCaptionPreview);
    mMenuView->AppendCheckItem(ID_SHOW_TIMELINES, sPaneCaptionTimelines);
    mMenuView->AppendSeparator();
    //
    wxMenu* menuViewDetails = new wxMenu();
    menuViewDetails->AppendCheckItem(ID_DETAILS_SHOW_ICONS, _("Show icons"));
    menuViewDetails->Check(ID_DETAILS_SHOW_ICONS, Config::get().read<bool>(Config::sPathDetailsShowIcons));
    menuViewDetails->AppendCheckItem(ID_DETAILS_SHOW_LABELS, _("Show labels"));
    menuViewDetails->Check(ID_DETAILS_SHOW_LABELS, Config::get().read<bool>(Config::sPathDetailsShowTitles));
    mMenuView->AppendSubMenu(menuViewDetails, _("Details"));
    //
    wxMenu* menuViewPreview = new wxMenu();
    menuViewPreview->AppendCheckItem(ID_PREVIEW_SHOW_BOUNDINGBOX, _("Show bounding box"));
    menuViewPreview->Check(ID_PREVIEW_SHOW_BOUNDINGBOX, Config::get().read<bool>(Config::sPathPreviewShowBoundingBox));
    mMenuView->AppendSubMenu(menuViewPreview, _("Preview"));
    //
    wxMenu* menuViewTimeline = new wxMenu();
    menuViewTimeline->AppendCheckItem(ID_TIMELINE_SNAP_CLIPS, _("Snap to clips"));
    menuViewTimeline->Check(ID_TIMELINE_SNAP_CLIPS, Config::get().read<bool>(Config::sPathTimelineSnapClips));
    menuViewTimeline->AppendCheckItem(ID_TIMELINE_SNAP_CURSOR, _("Snap to cursor"));
    menuViewTimeline->Check(ID_TIMELINE_SNAP_CURSOR, Config::get().read<bool>(Config::sPathTimelineSnapCursor));
    mMenuView->AppendSubMenu(menuViewTimeline, _("Timeline"));

    mMenuSequence = new wxMenu();

    wxMenu* menutools = new wxMenu();
    menutools->Append(wxID_PREFERENCES, _("Options"));

    mMenuWorkspace = new wxMenu();
    mMenuWorkspace->AppendCheckItem(ID_WORKSPACE_SHOW_CAPTIONS, _("Show captions"));
    mMenuWorkspace->Check(ID_WORKSPACE_SHOW_CAPTIONS,true);
    mMenuWorkspace->AppendSeparator();
    mMenuWorkspace->Append(ID_WORKSPACE_SAVE, _("Save"));
    mMenuWorkspace->Append(ID_WORKSPACE_LOAD, _("Load"));
    mMenuWorkspace->Append(ID_WORKSPACE_DELETE, _("Delete"));
    mMenuWorkspace->Append(ID_WORKSPACE_DELETEALL, _("Delete all"));
    mMenuWorkspace->AppendSeparator();
    mMenuWorkspace->Append(ID_WORKSPACE_DEFAULT, _("Restore default"));
    mMenuWorkspace->AppendSeparator();
    mMenuWorkspace->Append(ID_WORKSPACE_FULLSCREEN, _("Fullscreen") + "\t" + _("F11"));
    updateWorkspaceMenu();

    mMenuHelp = new wxMenu();
    mMenuHelp->AppendCheckItem(wxID_HELP, _("Help"));
    mMenuHelp->AppendSeparator();
    mMenuHelp->Append(ID_OPENLOGFILE, _("Open log file"));
    mMenuHelp->Append(ID_OPENCONFIGFILE, _("Open config file"));
    mMenuHelp->Append(ID_SUBMITBUG, _("Report bug/request feature"));
    mMenuHelp->AppendSeparator();
    mMenuHelp->Append(wxID_ABOUT, _("About..."));

    mMenuBar = new wxMenuBar();
    mMenuBar->Append(mMenuFile,     _("Project"));
    mMenuBar->Append(mMenuEdit,     _("Edit"));
    mMenuBar->Append(mMenuView,     _("View"));
    mMenuBar->Append(mMenuSequence, _("Sequence"));
    sSequenceMenuIndex = 3;
    mMenuBar->Append(menutools,    _("Tools"));
    mMenuBar->Append(mMenuWorkspace,_("Workspace"));
    if (Config::get().read<bool>(Config::sPathDebugShowCrashMenu))
    {
        mTestCrash = new util::TestCrash(this);
        mMenuBar->Append(mTestCrash->getMenu(), _("Crash"));
    }
    mMenuBar->Append(mMenuHelp,     _("Help"));

    SetMenuBar( mMenuBar );

    mMenuBar->EnableTop(sSequenceMenuIndex,false); // Disable sequence menu

    SetStatusBar(new StatusBar(this));

    mUiManager.SetManagedWindow(this);
    mUiManager.InsertPane(mHelp,
        wxAuiPaneInfo().
        Name(sPaneNameHelp).
        BestSize(wxSize(400,600)).
        MinSize(wxSize(100,100)).
        Layer(2).
        Right().
        Position(0).
        MaximizeButton().
        MinimizeButton().
        CaptionVisible(true).
        Caption(sPaneCaptionHelp).
        Hide()
        ); // Layer 2 to ensure that entire height is covered
    mUiManager.InsertPane(mProjectView,
        wxAuiPaneInfo().
        Name(sPaneNameProject).
        MinSize(wxSize(100,300)).
        Layer(1).
        Top().
        Position(0).
        MaximizeButton().
        MinimizeButton().
        CaptionVisible(true).
        Caption(sPaneCaptionProject)
        );
    mUiManager.InsertPane(mDetailsView,
        wxAuiPaneInfo().
        Name(sPaneNameDetails).
        MinSize(wxSize(100,300)).
        Layer(1).
        Top().
        Position(1).
        MaximizeButton().
        MinimizeButton().
        CaptionVisible(true).
        Caption(sPaneCaptionDetails)
        );
    mUiManager.InsertPane(mPreview,
        wxAuiPaneInfo().
        Name(sPaneNamePreview).
        MinSize(wxSize(100,300)).
        BestSize(wxSize(-1,800)).
        Layer(1).
        Top().
        Position(2).
        MaximizeButton().
        MinimizeButton().
        MaximizeButton().
        MinimizeButton().
        CaptionVisible(true).
        Caption(sPaneCaptionPreview)
        );
    mUiManager.InsertPane(mTimelinesView,
        wxAuiPaneInfo().
        Name(sPaneNameTimelines).
        MinSize(wxSize(100,100)).
        Resizable().
        Layer(1).
        Center().
        MaximizeButton().
        MinimizeButton().
        CaptionVisible(true).
        Caption(sPaneCaptionTimelines));
    mUiManager.SetFlags(wxAUI_MGR_ALLOW_FLOATING | wxAUI_MGR_ALLOW_ACTIVE_PANE | wxAUI_MGR_TRANSPARENT_HINT | wxAUI_MGR_LIVE_RESIZE );
    mUiManager.GetPane(mHelp).Hide();

    mUiManager.GetPane(sPaneNameProject).dock_proportion = 100000;
    mUiManager.GetPane(sPaneNameDetails).dock_proportion = 200000;
    mUiManager.GetPane(sPaneNamePreview).dock_proportion = 200000;
    mUiManager.GetPane(sPaneNameTimelines).dock_proportion = 200000;
    mUiManager.Update();

    mUiManager.Bind(wxEVT_AUI_PANE_CLOSE, &Window::onClosePane, this);
    mUiManager.Bind(wxEVT_AUI_PANE_RESTORE, &Window::onRestorePane, this);

    mDefaultPerspective = mUiManager.SavePerspective();

    wxString previous = Config::get().read<wxString>(Config::sPathWorkspacePerspectiveCurrent);
    if (!Config::get().read<bool>(Config::sPathTestCxxMode) && !previous.IsSameAs(""))
    {
        Config::get().write<wxString>(Config::sPathWorkspacePerspectiveCurrent, ""); // If this perspective causes problems, a restart will fix it. Upon closing the current perspective is saved again.
        mUiManager.LoadPerspective(previous);
        mUiManager.Update();
    }
    // Ensure that the panes are properly translated. When loading a 'previous' perspective, the pane titles are read from the perspective as well.
    // As a result, changing the language has no effect on pane titles. Explicitly setting the titles here ensures that the proper translations are used.
    mUiManager.GetPane(sPaneNameProject).Caption(sPaneCaptionProject);
    mUiManager.GetPane(sPaneNameDetails).Caption(sPaneCaptionDetails); // Also ensure that any specific details name (from a previous session) is replaced with the default.
    mUiManager.GetPane(sPaneNamePreview).Caption(sPaneCaptionPreview);
    mUiManager.GetPane(sPaneNameTimelines).Caption(sPaneCaptionTimelines);
    mUiManager.GetPane(sPaneNameHelp).Caption(sPaneCaptionHelp);
    updateViewMenu();

    Bind(model::EVENT_OPEN_PROJECT,     &Window::onOpenProject,     this);
    Bind(model::EVENT_CLOSE_PROJECT,    &Window::onCloseProject,    this);
    Bind(model::EVENT_RENAME_PROJECT,   &Window::onRenameProject,   this);

    Bind(wxEVT_MOVE,                    &Window::onMove,                this);
    Bind(wxEVT_SIZE,                    &Window::onSize,                this);
    Bind(wxEVT_MAXIMIZE,                &Window::onMaximize,            this);
    Bind(wxEVT_CLOSE_WINDOW,            &Window::onClose,               this);

    Bind(wxEVT_COMMAND_MENU_SELECTED,   &Window::onProperties,          this, wxID_PROPERTIES);
    Bind(wxEVT_COMMAND_MENU_SELECTED,   &Window::onNewFiles,            this, ID_NEW_FILES);
    Bind(wxEVT_COMMAND_MENU_SELECTED,   &Window::onNewAutoFolder,       this, ID_NEW_AUTOFOLDER);
    Bind(wxEVT_COMMAND_MENU_SELECTED,   &Window::onNewSequence,         this, ID_NEW_SEQUENCE);
    Bind(wxEVT_COMMAND_MENU_SELECTED,   &Window::onExit,                this, wxID_EXIT);

    Bind(wxEVT_COMMAND_MENU_SELECTED,   &wxDocManager::OnUndo,          GetDocumentManager(), wxID_UNDO);
    Bind(wxEVT_COMMAND_MENU_SELECTED,   &wxDocManager::OnRedo,          GetDocumentManager(), wxID_REDO);

    Bind(wxEVT_COMMAND_MENU_SELECTED,   &Window::onSnapClips,        this, ID_TIMELINE_SNAP_CLIPS);
    Bind(wxEVT_COMMAND_MENU_SELECTED,   &Window::onSnapCursor,       this, ID_TIMELINE_SNAP_CURSOR);
    Bind(wxEVT_COMMAND_MENU_SELECTED,   &Window::onShowBoundingBox,  this, ID_PREVIEW_SHOW_BOUNDINGBOX);
    Bind(wxEVT_COMMAND_MENU_SELECTED,   &Window::onShowLabels,       this, ID_DETAILS_SHOW_LABELS);
    Bind(wxEVT_COMMAND_MENU_SELECTED,   &Window::onShowIcons,        this, ID_DETAILS_SHOW_ICONS);
    Bind(wxEVT_COMMAND_MENU_SELECTED,   &Window::onShowProject,      this, ID_SHOW_PROJECT);
    Bind(wxEVT_COMMAND_MENU_SELECTED,   &Window::onShowDetails,      this, ID_SHOW_DETAILS);
    Bind(wxEVT_COMMAND_MENU_SELECTED,   &Window::onShowPreview,      this, ID_SHOW_PREVIEW);
    Bind(wxEVT_COMMAND_MENU_SELECTED,   &Window::onShowTimelines,    this, ID_SHOW_TIMELINES);

    Bind(wxEVT_COMMAND_MENU_SELECTED,   &Window::onOptions,          this, wxID_PREFERENCES);

    Bind(wxEVT_COMMAND_MENU_SELECTED,   &Window::onShowCaptions,       this, ID_WORKSPACE_SHOW_CAPTIONS);
    Bind(wxEVT_COMMAND_MENU_SELECTED,   &Window::onWorkspaceSave,      this, ID_WORKSPACE_SAVE);
    Bind(wxEVT_COMMAND_MENU_SELECTED,   &Window::onWorkspaceLoad,      this, ID_WORKSPACE_LOAD);
    Bind(wxEVT_COMMAND_MENU_SELECTED,   &Window::onWorkspaceDelete,    this, ID_WORKSPACE_DELETE);
    Bind(wxEVT_COMMAND_MENU_SELECTED,   &Window::onWorkspaceDeleteAll, this, ID_WORKSPACE_DELETEALL);
    Bind(wxEVT_COMMAND_MENU_SELECTED,   &Window::onWorkspaceDefault,   this, ID_WORKSPACE_DEFAULT);
    Bind(wxEVT_COMMAND_MENU_SELECTED,   &Window::onWorkspaceFullscreen,this, ID_WORKSPACE_FULLSCREEN);

    Bind(wxEVT_COMMAND_MENU_SELECTED,   &Window::onHelp,             this, wxID_HELP);
    Bind(wxEVT_COMMAND_MENU_SELECTED,   &Window::onLog,              this, ID_OPENLOGFILE);
    Bind(wxEVT_COMMAND_MENU_SELECTED,   &Window::onConfig,           this, ID_OPENCONFIGFILE);
    Bind(wxEVT_COMMAND_MENU_SELECTED,   &Window::onBug,              this, ID_SUBMITBUG);

    Bind(wxEVT_COMMAND_MENU_SELECTED,   &Window::onAbout,            this, wxID_ABOUT);

    Bind(wxEVT_DROP_FILES,              &Window::onDropFiles,        this);

    GetDocumentManager()->SetMaxDocsOpen(1);
    GetDocumentManager()->FileHistoryUseMenu(mMenuFile);
    GetDocumentManager()->FileHistoryLoad(Config::get());
    wxFileHistory* history{ GetDocumentManager()->GetFileHistory() };
    size_t n{ 0 };
    while (n < history->GetCount())
    {
        wxString file{ history->GetHistoryFile(n) };
        if (util::path::toFileName(file).FileExists())
        {
            // File exists, continue with the next file.
            ++n;
        }
        else
        {
            // File does not exist. Remove, and try the next file (which now has the same index).
            history->RemoveFileFromHistory(n);
        }
    }

    for (wxMenuItem* item : mMenuFile->GetMenuItems())
    {
        if (item->GetId() == wxID_CLOSE)
        {
            // This menu entry does not get an accelerator by default. Add Ctrl-W.
            item->SetItemLabel(item->GetItemLabelText() + "\t" + _("Ctrl") + "-W");
        }
    }
    for (wxMenuItem* item : mMenuHelp->GetMenuItems())
    {
        if (item->GetId() == wxID_HELP)
        {
            // This menu entry does not get an accelerator by default. Add Ctrl-H.
            item->SetItemLabel(item->GetItemLabelText() + "\t" + _("Ctrl") + "-H");
        }
    }

    if (Config::get().read<bool>(Config::sPathTestCxxMode))
    {
        wxSize screenSize = wxGetDisplaySize();
        wxSize winSize = GetSize();
        Move(screenSize.GetWidth() - winSize.GetWidth(),0);
    }
    else
    {
        int x = Config::get().read<long>(Config::sPathWorkspaceX);
        int y = Config::get().read<long>(Config::sPathWorkspaceY);
        int w = Config::get().read<long>(Config::sPathWorkspaceW);
        int h = Config::get().read<long>(Config::sPathWorkspaceH);
        bool m = Config::get().read<bool>(Config::sPathWorkspaceMaximized);

        if (x != -1 && y != -1 && w != -1 && h != -1)
        {
            SetSize(x,y,w,h);
        }
        if (m)
        {
            Maximize();
        }
    }
    Show();
}

void Window::init()
{
    if (CommandLine::get().EditFile)
    {
        GetDocumentManager()->CreateDocument(*CommandLine::get().EditFile);
    }
    else
    {
        bool autoload{ Config::get().read<bool>(Config::sPathProjectAutoLoadEnabled) };
        wxString mostRecent{ Config::get().read<wxString>(Config::sPathProjectLastOpened) };
        if (autoload && !mostRecent.IsEmpty())
        {
            Config::get().write<bool>(Config::sPathProjectAutoLoadEnabled, false); // If loading fails, then the next startup won't load by default.
            wxDocument* doc{ GetDocumentManager()->CreateDocument(mostRecent, wxDOC_SILENT) };
            if (doc != nullptr)
            {
                Config::get().write<bool>(Config::sPathProjectAutoLoadEnabled, true); // Loading ok, reset bool for next startup
            }
        }
    }
}

Window::~Window()
{
    mUiManager.Unbind(wxEVT_AUI_PANE_CLOSE, &Window::onClosePane, this);
    mUiManager.Unbind(wxEVT_AUI_PANE_RESTORE, &Window::onRestorePane, this);
    mUiManager.UnInit();

    Unbind(model::EVENT_OPEN_PROJECT,     &Window::onOpenProject,   this);
    Unbind(model::EVENT_CLOSE_PROJECT,    &Window::onCloseProject,  this);
    Unbind(model::EVENT_RENAME_PROJECT,   &Window::onRenameProject, this);

    Unbind(wxEVT_MOVE,                    &Window::onMove,                this);
    Unbind(wxEVT_SIZE,                    &Window::onSize,                this);
    Unbind(wxEVT_MAXIMIZE,                &Window::onMaximize,            this);
    Unbind(wxEVT_CLOSE_WINDOW,            &Window::onClose,               this);

    Unbind(wxEVT_COMMAND_MENU_SELECTED,   &Window::onNewFiles,            this, ID_NEW_FILES);
    Unbind(wxEVT_COMMAND_MENU_SELECTED,   &Window::onNewAutoFolder,       this, ID_NEW_AUTOFOLDER);
    Unbind(wxEVT_COMMAND_MENU_SELECTED,   &Window::onNewSequence,         this, ID_NEW_SEQUENCE);
    Unbind(wxEVT_COMMAND_MENU_SELECTED,   &Window::onExit,                this, wxID_EXIT);
    Unbind(wxEVT_COMMAND_MENU_SELECTED,   &Window::onProperties,          this, wxID_PROPERTIES);

    Unbind(wxEVT_COMMAND_MENU_SELECTED,   &wxDocManager::OnUndo,          GetDocumentManager(), wxID_UNDO);
    Unbind(wxEVT_COMMAND_MENU_SELECTED,   &wxDocManager::OnRedo,          GetDocumentManager(), wxID_REDO);

    Unbind(wxEVT_COMMAND_MENU_SELECTED,   &Window::onSnapClips,        this, ID_TIMELINE_SNAP_CLIPS);
    Unbind(wxEVT_COMMAND_MENU_SELECTED,   &Window::onSnapCursor,       this, ID_TIMELINE_SNAP_CURSOR);
    Unbind(wxEVT_COMMAND_MENU_SELECTED,   &Window::onShowBoundingBox,  this, ID_PREVIEW_SHOW_BOUNDINGBOX);
    Unbind(wxEVT_COMMAND_MENU_SELECTED,   &Window::onShowLabels,       this, ID_DETAILS_SHOW_LABELS);
    Unbind(wxEVT_COMMAND_MENU_SELECTED,   &Window::onShowIcons,        this, ID_DETAILS_SHOW_ICONS);
    Unbind(wxEVT_COMMAND_MENU_SELECTED,   &Window::onShowProject,      this, ID_SHOW_PROJECT);
    Unbind(wxEVT_COMMAND_MENU_SELECTED,   &Window::onShowDetails,      this, ID_SHOW_DETAILS);
    Unbind(wxEVT_COMMAND_MENU_SELECTED,   &Window::onShowPreview,      this, ID_SHOW_PREVIEW);
    Unbind(wxEVT_COMMAND_MENU_SELECTED,   &Window::onShowTimelines,    this, ID_SHOW_TIMELINES);

    Unbind(wxEVT_COMMAND_MENU_SELECTED,   &Window::onOptions,          this, wxID_PREFERENCES);

    Unbind(wxEVT_COMMAND_MENU_SELECTED,   &Window::onShowCaptions,       this, ID_WORKSPACE_SHOW_CAPTIONS);
    Unbind(wxEVT_COMMAND_MENU_SELECTED,   &Window::onWorkspaceSave,      this, ID_WORKSPACE_SAVE);
    Unbind(wxEVT_COMMAND_MENU_SELECTED,   &Window::onWorkspaceLoad,      this, ID_WORKSPACE_LOAD);
    Unbind(wxEVT_COMMAND_MENU_SELECTED,   &Window::onWorkspaceDelete,    this, ID_WORKSPACE_DELETE);
    Unbind(wxEVT_COMMAND_MENU_SELECTED,   &Window::onWorkspaceDeleteAll, this, ID_WORKSPACE_DELETEALL);
    Unbind(wxEVT_COMMAND_MENU_SELECTED,   &Window::onWorkspaceDefault,   this, ID_WORKSPACE_DEFAULT);
    Unbind(wxEVT_COMMAND_MENU_SELECTED,   &Window::onWorkspaceFullscreen,this, ID_WORKSPACE_FULLSCREEN);

    Unbind(wxEVT_COMMAND_MENU_SELECTED,   &Window::onHelp,             this, wxID_HELP);
    Unbind(wxEVT_COMMAND_MENU_SELECTED,   &Window::onLog,              this, ID_OPENLOGFILE);
    Unbind(wxEVT_COMMAND_MENU_SELECTED,   &Window::onConfig,           this, ID_OPENCONFIGFILE);
    Unbind(wxEVT_COMMAND_MENU_SELECTED,     &Window::onBug,              this, ID_SUBMITBUG);
    Unbind(wxEVT_COMMAND_MENU_SELECTED,   &Window::onAbout,            this, wxID_ABOUT);

    Unbind(wxEVT_DROP_FILES,              &Window::onDropFiles,        this);

    wxStatusBar* sb = GetStatusBar();
    SetStatusBar(0);
    sb->wxWindowBase::Destroy();

    delete mScheduler;
    delete mAudioTransitionFactory;
    delete mVideoTransitionFactory;

    delete mTestCrash; // Unbind the crash method
    setSequenceMenu(0,false); // Ensure destruction of sequenceMenu
    delete mProjectView;    // First, delete the referring windows.
    delete mTimelinesView;  // Fixed deletion order is required. ProjectView 'knows/uses' the timeline view,
    delete mPreview;        // the timeline view in turn 'knows/uses' the preview (specifically, the player).
    delete mDetailsView;
    delete mVisibleWorker;
    delete mInvisibleWorker;
    delete mDialog;
    //NOT: delete mDocTemplate;
    delete mDocManager;
}

//////////////////////////////////////////////////////////////////////////
// PROJECT EVENTS
//////////////////////////////////////////////////////////////////////////

void Window::ProcessModelEvent( wxEvent& event )
{
    GetEventHandler()->ProcessEvent(event);
}

void Window::QueueModelEvent( wxEvent* event )
{
    GetEventHandler()->QueueEvent(event);
}

void Window::onOpenProject(model::EventOpenProject &event )
{
    ASSERT_ZERO(mWatcher);
    // Needs an event loop (under wxGTK)
    // Therefore, creation is delayed until this moment.
    mWatcher = new model::FileWatcher();
    mMenuFile->Enable(ID_NEW_FILES,true);
    mMenuFile->Enable(ID_NEW_AUTOFOLDER,true);
    mMenuFile->Enable(ID_NEW_SEQUENCE,true);
    mMenuFile->Enable(wxID_PROPERTIES,true);
	mMenuEdit->Enable(wxID_CUT,true);
	mMenuEdit->Enable(wxID_COPY,true);
	mMenuEdit->Enable(wxID_PASTE,true);
    model::CommandProcessor::get().SetEditMenu(mMenuEdit); // Set menu for do/undo
    model::CommandProcessor::get().Initialize();
    GetDocumentManager()->AddFileToHistory(model::Project::get().GetFilename());
    GetDocumentManager()->FileHistorySave(Config::get());
    Config::get().Flush();
    updateTitle();
    mVisibleWorker->start();
    mInvisibleWorker->start();
    mProjectOpen = true;
    event.Skip();
}

void Window::onCloseProject(model::EventCloseProject &event )
{
    delete mWatcher;
    mWatcher = 0;
    mMenuFile->Enable(ID_NEW_FILES,false);
    mMenuFile->Enable(ID_NEW_AUTOFOLDER,false);
    mMenuFile->Enable(ID_NEW_SEQUENCE,false);
    mMenuFile->Enable(wxID_PROPERTIES,false);
	mMenuEdit->Enable(wxID_CUT,false);
	mMenuEdit->Enable(wxID_COPY,false);
	mMenuEdit->Enable(wxID_PASTE,false);
    SetTitle(mTitle); // Remove the title
    mVisibleWorker->abort();
    mInvisibleWorker->abort();
    mProjectOpen = false;
    event.Skip();
}

void Window::onRenameProject(model::EventRenameProject &event )
{
    GetDocumentManager()->AddFileToHistory(model::Project::get().GetFilename());
    GetDocumentManager()->FileHistorySave(Config::get());
    Config::get().Flush();
    updateTitle();
    event.Skip();
}

//////////////////////////////////////////////////////////////////////////
// GUI EVENTS
//////////////////////////////////////////////////////////////////////////

void Window::onMove(wxMoveEvent& event)
{
    if (!IsMaximized())
    {
        wxPoint p = GetScreenPosition();
        Config::get().write<bool>(Config::sPathWorkspaceMaximized,false);
        Config::get().write<long>(Config::sPathWorkspaceX,p.x); // Don't use event.GetPosition since that results in a slightly
        Config::get().write<long>(Config::sPathWorkspaceY,p.y); // moved window upon the next startup.
        Dialog::get().mScreenRect = GetScreenRect();
    }
    event.Skip();
}

void Window::onSize(wxSizeEvent& event)
{
    if (!IsMaximized())
    {
        Config::get().write<bool>(Config::sPathWorkspaceMaximized,false);
        Config::get().write<long>(Config::sPathWorkspaceW,event.GetSize().GetWidth());
        Config::get().write<long>(Config::sPathWorkspaceH,event.GetSize().GetHeight());
        Dialog::get().mScreenRect = GetScreenRect();
    }
    event.Skip();
}

void Window::onMaximize(wxMaximizeEvent& event)
{
    Config::get().write<bool>(Config::sPathWorkspaceMaximized,true);
    Dialog::get().mScreenRect = GetScreenRect();
    event.Skip();
}

void Window::onClose(wxCloseEvent& event)
{
    Config::get().write<wxString>(Config::sPathWorkspacePerspectiveCurrent, mUiManager.SavePerspective());
    Config::get().write<wxString>(Config::sPathProjectLastOpened, mProjectOpen ? util::path::toPath(model::Project::get().GetFilename()) : "");
    mVisibleWorker->abort();
    mInvisibleWorker->abort();
    event.Skip();
}

void Window::onClosePane(wxAuiManagerEvent& event)
{
    for (auto t : sMapMenuIdToPaneName)
    {
        if (t.second == event.GetPane()->name)
        {
            if (t.first == wxID_HELP)
            {
                mMenuHelp->Check(t.first, false);
            }
            else
            {
                mMenuView->Check(t.first, false);
            }
        }
    }
    event.Skip();
}

void Window::onRestorePane(wxAuiManagerEvent& event)
{
    for (auto t : sMapMenuIdToPaneName)
    {
        if (t.second == event.GetPane()->name)
        {
            if (t.first == wxID_HELP)
            {
                mMenuHelp->Check(t.first, true);
            }
            else
            {
                mMenuView->Check(t.first, true);
            }
        }
    }
    event.Skip();
}

//////////////////////////////////////////////////////////////////////////
// FILE MENU
//////////////////////////////////////////////////////////////////////////

void Window::onProperties(wxCommandEvent &event)
{
    DialogProjectProperties w(this);
    w.ShowModal();
    // NOT: event.Skip() -- see MenuHandler::onRenderSettings for rationale
}

void Window::onNewFiles(wxCommandEvent& event)
{
    mProjectView->onNewFileInRoot();
}

void Window::onNewAutoFolder(wxCommandEvent& event)
{
    mProjectView->onNewAutoFolderInRoot();
}

void Window::onNewSequence(wxCommandEvent& event)
{
    mProjectView->onNewSequenceInRoot();
}

void Window::onExit(wxCommandEvent &event)
{
    Close();
}

//////////////////////////////////////////////////////////////////////////
// VIEW MENU
//////////////////////////////////////////////////////////////////////////

void Window::onSnapClips(wxCommandEvent& event)
{
    Config::get().write<bool>(Config::sPathTimelineSnapClips, event.IsChecked());
    event.Skip();
}

void Window::onSnapCursor(wxCommandEvent& event)
{
    Config::get().write<bool>(Config::sPathTimelineSnapCursor, event.IsChecked());
    event.Skip();
}

void Window::onShowBoundingBox(wxCommandEvent& event)
{
    Config::get().write<bool>(Config::sPathPreviewShowBoundingBox, event.IsChecked());
    event.Skip();
}

void Window::onShowLabels(wxCommandEvent& event)
{
    Config::get().write<bool>(Config::sPathDetailsShowTitles, event.IsChecked());
    event.Skip();
}

void Window::onShowIcons(wxCommandEvent& event)
{
    Config::get().write<bool>(Config::sPathDetailsShowIcons, event.IsChecked());
    event.Skip();
}

void Window::onShowProject(wxCommandEvent& event)
{
    togglePane(sPaneNameProject);
    event.Skip();
}

void Window::onShowDetails(wxCommandEvent& event)
{
    togglePane(sPaneNameDetails);
    event.Skip();
}

void Window::onShowPreview(wxCommandEvent& event)
{
    togglePane(sPaneNamePreview);
    event.Skip();
}

void Window::onShowTimelines(wxCommandEvent& event)
{
    togglePane(sPaneNameTimelines);
    event.Skip();
}

//////////////////////////////////////////////////////////////////////////
// TOOLS MENU
//////////////////////////////////////////////////////////////////////////

void Window::onOptions(wxCommandEvent& event)
{
    DialogOptions w(this);
    w.ShowModal();
    // NOT: event.Skip() -- see MenuHandler::onRenderSettings for rationale
}

//////////////////////////////////////////////////////////////////////////
// WORKSPACE MENU
//////////////////////////////////////////////////////////////////////////

void Window::onShowCaptions(wxCommandEvent& event)
{
    for (unsigned int i = 0; i < mUiManager.GetAllPanes().Count(); ++i)
    {
        mUiManager.GetAllPanes().Item(i).CaptionVisible(event.IsChecked());
    }
    mUiManager.Update();
    event.Skip();
}

void Window::onWorkspaceSave(wxCommandEvent& event)
{
    wxString name = Dialog::get().getText(_("Enter workspace name"),_("Enter name for the saved workspace.\nIf this is an existing name it is overwritten."), "");
    if (!name.IsEmpty())
    {
        Config::get().addWorkspacePerspective(name,mUiManager.SavePerspective());
        updateWorkspaceMenu();
    }
    event.Skip();
}

wxString selectWorkspace(wxString text)
{
    Config::Perspectives perspectives{ Config::get().getWorkspacePerspectives() };
    wxStrings entries;
    for ( Config::Perspectives::value_type name_perspective : perspectives )
    {
        entries.emplace_back(name_perspective.first);
    }
    wxString result = Dialog::get().getComboText(_("Select workspace"),text, entries);
    return result;
}

void Window::onWorkspaceLoad(wxCommandEvent& event)
{
    wxString name = selectWorkspace(_("Select workspace to be restored."));
    if (!name.IsEmpty())
    {
        Config::Perspectives perspectives{ Config::get().getWorkspacePerspectives() };
        ASSERT_MAP_CONTAINS(perspectives,name);
        bool success = mUiManager.LoadPerspective(perspectives[name]);
        mUiManager.Update();
        if (!success)
        {
            gui::Dialog::get().getConfirmation(_("Load workspace failed"),_("Something went wrong while trying to load the saved perspective. Sorry."));
        }
    }
    event.Skip();
}

void Window::onWorkspaceDelete(wxCommandEvent& event)
{
    wxString name = selectWorkspace(_("Select workspace to be removed."));
    if (!name.IsEmpty())
    {
        Config::Perspectives perspectives{ Config::get().getWorkspacePerspectives() };
        ASSERT_MAP_CONTAINS(perspectives,name);
        Config::get().removeWorkspacePerspective(name);
        updateWorkspaceMenu();
    }
    event.Skip();
}

void Window::onWorkspaceDeleteAll(wxCommandEvent& event)
{
    if (wxYES == Dialog::get().getConfirmation(_("Remove all saved workspaces"), _("This will remove all saved workspace layouts which cannot be undone.\nAre you sure?"), wxYES | wxNO))
    {
        Config::get().removeAllWorkspacePerspectives();
        updateWorkspaceMenu();
    }
    event.Skip();
}

void Window::onWorkspaceDefault(wxCommandEvent& event)
{
    mUiManager.LoadPerspective(mDefaultPerspective);
    event.Skip();
}

void Window::onWorkspaceFullscreen(wxCommandEvent& event)
{
    static boost::posix_time::ptime previous = boost::posix_time::ptime(boost::date_time::min_date_time);
    boost::posix_time::ptime t = boost::posix_time::microsec_clock::local_time();
    boost::posix_time::time_duration diff = t - previous;
    if (diff.total_milliseconds() > 50)
    {
        // Under wxGTK, sometimes the move from fullscreen to normal does not work.
        // This is caused by multiple events being triggered upon one key press.
        // Given that this is a toggle, two consecutive events negate each other.
        // Therefore, consecutive events within 50ms are ignored.
        ShowFullScreen(!IsFullScreen(), wxFULLSCREEN_NOTOOLBAR | wxFULLSCREEN_NOBORDER | wxFULLSCREEN_NOCAPTION);
    }
    previous = t;
    event.Skip();
}

//////////////////////////////////////////////////////////////////////////
// HELP MENU
//////////////////////////////////////////////////////////////////////////

void Window::onHelp(wxCommandEvent& event)
{
    togglePane(sPaneNameHelp);
    event.Skip();
}

void Window::onLog(wxCommandEvent& event)
{

    if (!wxLaunchDefaultApplication(util::path::getLogFilePath().GetLongPath()))
    {
        // TRANSLATORS: %s == Path to file that cannot be opened
        Dialog::get().getConfirmation(_("Failed to open file"), wxString::Format(_("Failed to open '%s'"), util::path::getLogFilePath().GetLongPath()));
    }
    event.Skip();
}

void Window::onConfig(wxCommandEvent& event)
{
#if !(wxUSE_PRINTF_POS_PARAMS == 1)
#error wxUSE_PRINTF_POS_PARAMS must equal 1
#endif
    static bool shown = false;
    if (!shown)
    {
        Dialog::get().getConfirmation(_("Attention"),
            wxString::Format(_("Note that changing this file will not update the gui.\n"
            "Changed settings are applied only after restarting %1$s.\n"
            "Note that %1$s, when closing, also updates the file. Therefore, always\n"
            "close %1$s first, before editing and saving this file in a text editor.\n"
            "\n\n"
            "Incorrectly changed settings may cause %1$s to crash upon startup.\n"
            "If that happens, delete the file from disk (or make the file empty) and restart %1$s.\n"), CommandLine::get().ExeName));
        shown = true;
    }
    if (!wxLaunchDefaultApplication(util::path::getConfigFilePath().GetLongPath()))
    {
        // TRANSLATORS: %s == Path to file that cannot be opened
        Dialog::get().getConfirmation(_("Failed to open file"), wxString::Format(_("Failed to open '%s'"), util::path::getConfigFilePath().GetLongPath()));
    }
    event.Skip();
}

void Window::onBug(wxCommandEvent& event)
{
    Dialog::get().getDebugReport(false,false);
    event.Skip();
}

void Window::onAbout(wxCommandEvent& event)
{
    DialogAbout* dialog = new DialogAbout();
    dialog->ShowModal();
    delete dialog;
    // NOT: event.Skip() -- see MenuHandler::onRenderSettings for rationale
}

//////////////////////////////////////////////////////////////////////////
// DROP FILES EVENTS
//////////////////////////////////////////////////////////////////////////

void Window::onDropFiles(wxDropFilesEvent& event)
{
    wxStrings filenames;
    for (int i = 0; i < event.GetNumberOfFiles(); ++i)
    {
        filenames.emplace_back(event.GetFiles()[i]);
    }
    // Required for some operations (determining file lengths in the file analyzer)
    model::PropertiesPtr tempProperties = isProjectOpened() ? nullptr : boost::make_shared<model::Properties>();
    boost::shared_ptr<model::FileAnalyzer> analyzer{ boost::make_shared < model::FileAnalyzer > ( filenames, this ) };
    tempProperties.reset();

    if (analyzer->isProjectOnly())
    {
        mDocManager->CreateDocument(filenames.front());
    }
    else if (isProjectOpened())
    {
        analyzer->addNodesToProjectView();
    }
    else
    {
        DialogNewProject::setDroppedFiles(filenames);
        GetEventHandler()->QueueEvent(new wxCommandEvent(wxEVT_COMMAND_MENU_SELECTED,wxID_NEW));
    }
    // NOT: event.Skip() -- only handled here.
}


//////////////////////////////////////////////////////////////////////////
// WIDGET HANDLING
//////////////////////////////////////////////////////////////////////////

TimelinesView& Window::getTimeLines()
{
    return *mTimelinesView;
}

Preview& Window::getPreview()
{
    return *mPreview;
}

DetailsView& Window::getDetailsView()
{
    return *mDetailsView;
}

wxAuiManager& Window::getUiManager()
{
    return mUiManager;
}

void Window::triggerLayout()
{
    mUiManager.Update();
}

//////////////////////////////////////////////////////////////////////////
// ENABLING/DISABLING MENUS
//////////////////////////////////////////////////////////////////////////

void Window::setSequenceMenu(wxMenu* menu, bool enabled)
{
    bool enable{ enabled };
    if (menu == nullptr)
    {
        menu = mMenuSequence;
        enable = false;
    }
    if (mMenuBar->GetMenu(sSequenceMenuIndex) != menu)
    {
        // Only in case of changes. Otherwise wxWidgets asserts.
        mMenuBar->Replace(sSequenceMenuIndex, menu, _("Sequence"));
    }
    mMenuBar->EnableTop(sSequenceMenuIndex,enable);
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

void Window::setAdditionalTitle(const wxString& title)
{
    SetTitle(mTitle + ": " + title);
}

bool Window::isDialogOpen() const
{
    return mDialogOpen;
}

void Window::setDialogOpen(bool open)
{
    mDialogOpen = open;
}

bool Window::isProjectOpened() const
{
    ASSERT(wxThread::IsMain());
    return mProjectOpen;
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void Window::updateViewMenu()
{
    for (auto t : sMapMenuIdToPaneName)
    {
        if (t.first == wxID_HELP) { continue; } // Skip help menu
        wxAuiPaneInfo& pane = mUiManager.GetPane(t.second);
        ASSERT(pane.IsOk());
        mMenuView->Check(t.first, pane.IsShown());
    }
}

void Window::updateWorkspaceMenu()
{
    bool enable{ !Config::get().getWorkspacePerspectives().empty() };
    mMenuWorkspace->Enable(ID_WORKSPACE_LOAD, enable);
    mMenuWorkspace->Enable(ID_WORKSPACE_DELETE, enable);
    mMenuWorkspace->Enable(ID_WORKSPACE_DELETEALL, enable);
}

void Window::updateTitle()
{
    if (GetDocumentManager()->GetCurrentDocument() != 0)
    {
        SetTitle(model::Project::get().getRoot()->getName() + " - " + mTitle);
    }
    else
    {
        SetTitle(mTitle);
    }
}

void Window::togglePane(const wxString& title)
{
    wxAuiPaneInfo& pane = mUiManager.GetPane(title);
    ASSERT(pane.IsOk()); // Lookup must succeed
    pane.Show(!pane.IsShown());
    mUiManager.Update();
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void Window::serialize(Archive & ar, const unsigned int version)
{
    try
    {
        ar & boost::serialization::make_nvp("projectview",*mProjectView);
        ar & boost::serialization::make_nvp("timelinesview",*mTimelinesView);
    }
    catch (boost::exception &e)                  { VAR_ERROR(boost::diagnostic_information(e)); throw; }
    catch (std::exception& e)                    { VAR_ERROR(e.what());                         throw; }
    catch (...)                                  { LOG_ERROR;                                   throw; }
}
template void Window::serialize<boost::archive::xml_oarchive>(boost::archive::xml_oarchive& ar, const unsigned int archiveVersion);
template void Window::serialize<boost::archive::xml_iarchive>(boost::archive::xml_iarchive& ar, const unsigned int archiveVersion);

} // namespace

namespace model {
    // static
    IView& IView::getView()
    {
        return gui::Window::get();
    }

    template<class Archive>
    void IView::serialize(Archive & ar, const unsigned int version)
    {
        try
        {
            ar & boost::serialization::make_nvp("window",static_cast<gui::Window&>(*this));
        }
        catch (boost::exception &e)                  { VAR_ERROR(boost::diagnostic_information(e)); throw; }
        catch (std::exception& e)                    { VAR_ERROR(e.what());                         throw; }
        catch (...)                                  { LOG_ERROR;                                   throw; }
    }
    template void IView::serialize<boost::archive::xml_oarchive>(boost::archive::xml_oarchive& ar, const unsigned int archiveVersion);
    template void IView::serialize<boost::archive::xml_iarchive>(boost::archive::xml_iarchive& ar, const unsigned int archiveVersion);
}
