// Copyright 2013,2014 Eric Raijmakers.
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
#include "Config.h"
#include "Dialog.h"
#include "DialogAbout.h"
#include "DialogOptions.h"
#include "Help.h"
#include "ids.h"
#include "Layout.h"
#include "Node.h"
#include "Preview.h"
#include "Project.h"
#include "ProjectEvent.h"
#include "ProjectView.h"
#include "StatusBar.h"
#include "TimelinesView.h"
#include "UtilLog.h"
#include "UtilTestCrash.h"
#include "VideoTransitionFactory.h"
#include "Watcher.h"
#include "Worker.h"

namespace gui {

const wxString Window::sPaneNameHelp("Help");
const wxString Window::sPaneNameProject("Project");
const wxString Window::sPaneNameDetails("Details");
const wxString Window::sPaneNamePreview("Preview");
const wxString Window::sPaneNameTimelines("Timelines");

const std::map<int, wxString> Window::sMapMenuIdToPaneName = boost::assign::map_list_of
    (static_cast<int>(ID_SHOW_PROJECT),     Window::sPaneNameProject)
    (static_cast<int>(ID_SHOW_DETAILS),     Window::sPaneNameDetails)
    (static_cast<int>(ID_SHOW_PREVIEW),     Window::sPaneNamePreview)
    (static_cast<int>(ID_SHOW_TIMELINES),   Window::sPaneNameTimelines)
    (static_cast<int>(wxID_HELP),           Window::sPaneNameHelp);

const wxString sPaneCaptionHelp(_("Help"));
const wxString sPaneCaptionProject(_("Project"));
const wxString sPaneCaptionDetails(_("Details"));
const wxString sPaneCaptionPreview(_("Preview"));
const wxString sPaneCaptionTimelines(_("Timelines"));

//////////////////////////////////////////////////////////////////////////
// HELPER CLASSES
//////////////////////////////////////////////////////////////////////////

class ViewHelper
    :   public wxView
{
public:
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
static const wxString sTitle(_("Vidiot"));

Window::Window()
    :   wxDocParentFrame(new wxDocManager(), 0, wxID_ANY, sTitle, wxDefaultPosition, wxSize(1000,700))
    ,	mDocTemplate(new wxDocTemplate(GetDocumentManager(), _("Vidiot files"), "*.vid", "", "vid", _("Vidiot Project"), _("Vidiot Project View"), CLASSINFO(model::Project), CLASSINFO(ViewHelper)))
    ,   mDialog(new Dialog())
    ,   mLayout(new gui::Layout())
    ,   mWatcher(0)
    ,   mWorker(0)
    ,   mPreview(0)
    ,   mDetailsView(0)
    ,   mTimelinesView(0)
    ,   mProjectView(0)
    ,   mMenuBar(0)
    ,   mMenuEdit(0)
    ,   mMenuSequence(0)
    ,   mTestCrash(new util::TestCrash(this))
    ,   mAudioTransitionFactory(new model::audio::AudioTransitionFactory())
    ,   mVideoTransitionFactory(new model::video::VideoTransitionFactory())
{
    // Construction not done in constructor list due to dependency on sCurrent
    mWorker         = new worker::Worker();
    mWatcher        = new Watcher();
    mPreview        = new Preview(this); // Must be opened before timelinesview for the case of autoloading with open sequences/timelines
    mDetailsView    = new DetailsView(this);
    mTimelinesView  = new TimelinesView(this);
    mProjectView    = new ProjectView(this);
    mHelp           = new Help(this);

    wxIconBundle icons;
    icons.AddIcon(Config::getExeDir() + "\\icons\\movie_all.ico"); // Icon in title bar of window
    SetIcons(icons);

    wxMenu* menufile = new wxMenu();
    menufile->Append(wxID_NEW);
    menufile->Append(wxID_OPEN);
    menufile->Append(wxID_CLOSE);
    //	menufile->Append(wxID_REVERT);
    menufile->Append(wxID_SAVE);
    menufile->Append(wxID_SAVEAS);
    menufile->AppendSeparator();
    menufile->Append(wxID_EXIT, _("E&xit"), _("Select exit to end the application."));

    mMenuEdit = new wxMenu();
    mMenuEdit->Append(wxID_UNDO);
    mMenuEdit->Append(wxID_REDO);
    mMenuEdit->AppendSeparator();
    mMenuEdit->Append(wxID_CUT);
    mMenuEdit->Append(wxID_COPY);
    mMenuEdit->Append(wxID_PASTE);

    wxMenu* menuview = new wxMenu();
    menuview->AppendCheckItem(ID_SNAP_CLIPS, _("Snap to clips"), _("Check this item to ensure that operations in the timeline 'snap' to adjacent clip boundaries."));
    menuview->Check(ID_SNAP_CLIPS, Config::ReadBool(Config::sPathSnapClips));
    menuview->AppendCheckItem(ID_SNAP_CURSOR, _("Snap to cursor"), _("Check this item to ensure that operations in the timeline 'snap' to the cursor position."));
    menuview->Check(ID_SNAP_CURSOR, Config::ReadBool(Config::sPathSnapCursor));
    menuview->AppendSeparator();
    menuview->AppendCheckItem(ID_SHOW_BOUNDINGBOX, _("Show bounding box"), _("Show the bounding box of the generated video in the preview window."));
    menuview->Check(ID_SHOW_BOUNDINGBOX, Config::ReadBool(Config::sPathShowBoundingBox));
    menuview->AppendSeparator();
    menuview->AppendCheckItem(ID_SHOW_PROJECT, sPaneCaptionProject, _("Show/hide the project view pane."));
    menuview->AppendCheckItem(ID_SHOW_DETAILS, sPaneCaptionDetails, _("Show/hide the timeline details pane."));
    menuview->AppendCheckItem(ID_SHOW_PREVIEW, sPaneCaptionPreview, _("Show/hide the timeline preview pane."));
    menuview->AppendCheckItem(ID_SHOW_TIMELINES, sPaneCaptionTimelines, _("Show/hide the timelines pane."));

    mMenuSequence = new wxMenu();

    wxMenu* menutools = new wxMenu();
    menutools->Append(wxID_PREFERENCES, _("&Options"), _("Open the options dialog."));

    mMenuWorkspace = new wxMenu();
    mMenuWorkspace->AppendCheckItem(ID_WORKSPACE_SHOW_CAPTIONS, _("Show captions"), _("Toggle this option to hide/show the captions on the workspace windows."));
    mMenuWorkspace->Check(ID_WORKSPACE_SHOW_CAPTIONS,true);
    mMenuWorkspace->AppendSeparator();
    mMenuWorkspace->Append(ID_WORKSPACE_SAVE, _("Save"), _("Save the current workspace layout."));
    mMenuWorkspace->Append(ID_WORKSPACE_LOAD, _("Load"), _("Load a previously saved workspace layout."));
    mMenuWorkspace->Append(ID_WORKSPACE_DELETE, _("Delete"), _("Select a workspace layout to be deleted."));
    mMenuWorkspace->Append(ID_WORKSPACE_DELETEALL, _("Delete all"), _("Delete all saved workspace layouts."));
    mMenuWorkspace->AppendSeparator();
    mMenuWorkspace->Append(ID_WORKSPACE_DEFAULT, ("Restore default"), _("Restore the original workspace layout."));
    mMenuWorkspace->AppendSeparator();
    mMenuWorkspace->Append(ID_WORKSPACE_FULLSCREEN, ("Fullscreen\tF11"), _("Toggle between windowed and fullscreen mode."));
    updateWorkspaceMenu();

    wxMenu* menuhelp = new wxMenu();
    menuhelp->Append(wxID_HELP, _("Help"), _("Open the help.") );
    menuhelp->AppendSeparator();
    menuhelp->Append(ID_OPENLOGFILE, _("Open log file"), _("Use the default application associated with .txt files to open the log file."));
    menuhelp->Append(ID_OPENCONFIGFILE, _("Open config file"), _("Use the default application associated with .ini files to open the config file."));
    menuhelp->AppendSeparator();
    menuhelp->Append(wxID_ABOUT, _("&About..."), _("Show the about dialog."));

    mMenuBar = new wxMenuBar();
    mMenuBar->Append(menufile,     _("&File"));
    mMenuBar->Append(mMenuEdit,     _("&Edit"));
    mMenuBar->Append(menuview,     _("&View"));
    mMenuBar->Append(mMenuSequence, _("&Sequence"));
    sSequenceMenuIndex = 3;
    mMenuBar->Append(menutools,    _("&Tools"));
    mMenuBar->Append(mMenuWorkspace,_("&Workspace"));
    if (Config::ReadBool(Config::sPathDebugShowCrashMenu))
    {
        mMenuBar->Append(mTestCrash->getMenu(),     _("&Crash"));
    }

    mMenuBar->Append(menuhelp,     _("&Help"));

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

    mDefaultPerspective = mUiManager.SavePerspective();

    wxString previous = Config::ReadString(Config::sPathWorkspacePerspectiveCurrent);
    if (!Config::ReadBool(Config::sPathTest) && !previous.IsSameAs(""))
    {
        Config::WriteString(Config::sPathWorkspacePerspectiveCurrent, ""); // If this perspective causes problems, a restart will fix it. Upon closing the current perspective is saved again.
        mUiManager.LoadPerspective(previous);
        mUiManager.Update();
    }

    for (auto t : sMapMenuIdToPaneName)
    {
        if (t.first == wxID_HELP) { continue; } // Skip help menu
        wxAuiPaneInfo& pane = mUiManager.GetPane(t.second);
        ASSERT(pane.IsOk());
        menuview->Check(t.first, pane.IsShown());
    }

    Bind(model::EVENT_OPEN_PROJECT,     &Window::onOpenProject,     this);
    Bind(model::EVENT_CLOSE_PROJECT,    &Window::onCloseProject,    this);
    Bind(model::EVENT_RENAME_PROJECT,   &Window::onRenameProject,   this);

    Bind(wxEVT_MOVE,                    &Window::onMove,                this);
    Bind(wxEVT_SIZE,                    &Window::onSize,                this);
    Bind(wxEVT_MAXIMIZE,                &Window::onMaximize,            this);
    Bind(wxEVT_CLOSE_WINDOW,            &Window::onClose,               this);

    Bind(wxEVT_COMMAND_MENU_SELECTED,   &Window::onExit,                this, wxID_EXIT);

    Bind(wxEVT_COMMAND_MENU_SELECTED,   &wxDocManager::OnUndo,          GetDocumentManager(), wxID_UNDO);
    Bind(wxEVT_COMMAND_MENU_SELECTED,   &wxDocManager::OnRedo,          GetDocumentManager(), wxID_REDO);

    Bind(wxEVT_COMMAND_MENU_SELECTED,   &Window::onSnapClips,        this, ID_SNAP_CLIPS);
    Bind(wxEVT_COMMAND_MENU_SELECTED,   &Window::onSnapCursor,       this, ID_SNAP_CURSOR);
    Bind(wxEVT_COMMAND_MENU_SELECTED,   &Window::onShowBoundingBox,  this, ID_SHOW_BOUNDINGBOX);
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

    Bind(wxEVT_COMMAND_MENU_SELECTED,   &Window::onAbout,            this, wxID_ABOUT);

    GetDocumentManager()->SetMaxDocsOpen(1);
    GetDocumentManager()->FileHistoryUseMenu(menufile);
    GetDocumentManager()->FileHistoryLoad(*wxConfigBase::Get());

    if (Config::ReadBool(Config::sPathTest))
    {
        wxSize screenSize = wxGetDisplaySize();
        wxSize winSize = GetSize();
        Move(screenSize.GetWidth() - winSize.GetWidth(),0);
    }
    else
    {
        int x = Config::ReadLong(Config::sPathWindowX);
        int y = Config::ReadLong(Config::sPathWindowY);
        int w = Config::ReadLong(Config::sPathWindowW);
        int h = Config::ReadLong(Config::sPathWindowH);
        bool m = Config::ReadBool(Config::sPathWindowMaximized);

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

    if (!Log::isEnabled())
    {
        if (wxNO == Dialog::get().getConfirmation( _("Log file error"), _("Could not open the log file for writing. Continue running?"), wxYES | wxNO))
        {
            wxExit();
        }
    }
}

void Window::init()
{
    if (CommandLine::get().EditFile)
    {
        GetDocumentManager()->CreateDocument(*CommandLine::get().EditFile);
    }
    else
    {
        if (Config::ReadBool(Config::sPathAutoLoadEnabled))
        {
            wxFileHistory* history = GetDocumentManager()->GetFileHistory();
            if (history->GetCount() > 0)
            {
                GetDocumentManager()->CreateDocument(history->GetHistoryFile(0), wxDOC_SILENT);
            }
        }
    }
}

Window::~Window()
{
    Unbind(model::EVENT_OPEN_PROJECT,     &Window::onOpenProject,   this);
    Unbind(model::EVENT_CLOSE_PROJECT,    &Window::onCloseProject,  this);
    Unbind(model::EVENT_RENAME_PROJECT,   &Window::onRenameProject, this);

    Unbind(wxEVT_MOVE,                    &Window::onMove,                this);
    Unbind(wxEVT_SIZE,                    &Window::onSize,                this);
    Unbind(wxEVT_MAXIMIZE,                &Window::onMaximize,            this);
    Unbind(wxEVT_CLOSE_WINDOW,            &Window::onClose,               this);

    Unbind(wxEVT_COMMAND_MENU_SELECTED,   &Window::onExit,                this, wxID_EXIT);

    Unbind(wxEVT_COMMAND_MENU_SELECTED,   &wxDocManager::OnUndo,          GetDocumentManager(), wxID_UNDO);
    Unbind(wxEVT_COMMAND_MENU_SELECTED,   &wxDocManager::OnRedo,          GetDocumentManager(), wxID_REDO);

    Unbind(wxEVT_COMMAND_MENU_SELECTED,   &Window::onSnapClips,        this, ID_SNAP_CLIPS);
    Unbind(wxEVT_COMMAND_MENU_SELECTED,   &Window::onSnapCursor,       this, ID_SNAP_CURSOR);
    Unbind(wxEVT_COMMAND_MENU_SELECTED,   &Window::onShowBoundingBox,  this, ID_SHOW_BOUNDINGBOX);
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
    Unbind(wxEVT_COMMAND_MENU_SELECTED,   &Window::onAbout,            this, wxID_ABOUT);

    mUiManager.UnInit();

    wxStatusBar* sb = GetStatusBar();
    SetStatusBar(0);
    sb->wxWindowBase::Destroy();

    delete mAudioTransitionFactory;
    delete mVideoTransitionFactory;

    delete mTestCrash; // Unbind the crash method
    setSequenceMenu(0,false); // Ensure destruction of sequenceMenu
    delete mProjectView;    // First, delete the referring windows.
    delete mTimelinesView;  // Fixed deletion order is required. ProjectView 'knows/uses' the timeline view,
    delete mPreview;        // the timeline view in turn 'knows/uses' the preview (specifically, the player).
    delete mDetailsView;
    delete mWatcher;
    delete mWorker;
    delete mDialog;
    delete mLayout;
    //NOT: delete mDocTemplate;
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

void Window::onOpenProject( model::EventOpenProject &event )
{
    GetDocumentManager()->GetCurrentDocument()->GetCommandProcessor()->SetEditMenu(mMenuEdit); // Set menu for do/undo
    GetDocumentManager()->GetCurrentDocument()->GetCommandProcessor()->Initialize();
    GetDocumentManager()->AddFileToHistory(model::Project::get().GetFilename());
    GetDocumentManager()->FileHistorySave(*wxConfigBase::Get());
    wxConfigBase::Get()->Flush();
    updateTitle();
    mWorker->start();
    event.Skip();
}

void Window::onCloseProject( model::EventCloseProject &event )
{
    updateTitle();
    mWorker->abort();
    event.Skip();
}

void Window::onRenameProject( model::EventRenameProject &event )
{
    GetDocumentManager()->AddFileToHistory(model::Project::get().GetFilename());
    GetDocumentManager()->FileHistorySave(*wxConfigBase::Get());
    wxConfigBase::Get()->Flush();
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
        Config::WriteBool(Config::sPathWindowMaximized,false);
        Config::WriteLong(Config::sPathWindowX,p.x); // Don't use event.GetPosition since that results in a slightly
        Config::WriteLong(Config::sPathWindowY,p.y); // moved window upon the next startup.
        Dialog::get().mScreenRect = GetScreenRect();
    }
    event.Skip();
}

void Window::onSize(wxSizeEvent& event)
{
    if (!IsMaximized())
    {
        Config::WriteBool(Config::sPathWindowMaximized,false);
        Config::WriteLong(Config::sPathWindowW,event.GetSize().GetWidth());
        Config::WriteLong(Config::sPathWindowH,event.GetSize().GetHeight());
        Dialog::get().mScreenRect = GetScreenRect();
    }
    event.Skip();
}

void Window::onMaximize(wxMaximizeEvent& event)
{
    Config::WriteBool(Config::sPathWindowMaximized,true);
    Dialog::get().mScreenRect = GetScreenRect();
    event.Skip();
}

void Window::onClose(wxCloseEvent& event)
{
    Config::WriteString(Config::sPathWorkspacePerspectiveCurrent, mUiManager.SavePerspective());
    mWorker->abort();
    event.Skip();
}

//////////////////////////////////////////////////////////////////////////
// FILE MENU
//////////////////////////////////////////////////////////////////////////

void Window::onExit(wxCommandEvent &event)
{
    Close();
}

//////////////////////////////////////////////////////////////////////////
// VIEW MENU
//////////////////////////////////////////////////////////////////////////

void Window::onSnapClips(wxCommandEvent& event)
{
    Config::WriteBool(Config::sPathSnapClips, event.IsChecked());
    event.Skip();
}

void Window::onSnapCursor(wxCommandEvent& event)
{
    Config::WriteBool(Config::sPathSnapCursor, event.IsChecked());
    event.Skip();
}

void Window::onShowBoundingBox(wxCommandEvent& event)
{
    Config::WriteBool(Config::sPathShowBoundingBox, event.IsChecked());
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
        Config::WorkspacePerspectives::add(name,mUiManager.SavePerspective());
        updateWorkspaceMenu();
    }
    event.Skip();
}

wxString selectWorkspace(wxString text)
{
    Config::Perspectives perspectives = Config::WorkspacePerspectives::get();
    std::list<wxString> entries;
    for ( Config::Perspectives::value_type name_perspective : perspectives )
    {
        entries.push_back(name_perspective.first);
    }
    wxString result = Dialog::get().getComboText(_("Select workspace"),text, entries);
    return result;
}

void Window::onWorkspaceLoad(wxCommandEvent& event)
{
    wxString name = selectWorkspace(_("Select workspace to be restored."));
    if (!name.IsEmpty())
    {
        Config::Perspectives perspectives = Config::WorkspacePerspectives::get();
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
        Config::Perspectives perspectives = Config::WorkspacePerspectives::get();
        ASSERT_MAP_CONTAINS(perspectives,name);
        Config::WorkspacePerspectives::remove(name);
        updateWorkspaceMenu();
    }
    event.Skip();
}

void Window::onWorkspaceDeleteAll(wxCommandEvent& event)
{
    if (wxYES == Dialog::get().getConfirmation(_("Remove all saved workspaces"), _("This will remove all saved workspace layouts which cannot be undone.\nAre you sure?"), wxYES | wxNO))
    {
        Config::WorkspacePerspectives::removeAll();
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
    bool fullscreen = !IsFullScreen();
    ShowFullScreen(fullscreen, wxFULLSCREEN_NOTOOLBAR | wxFULLSCREEN_NOBORDER | wxFULLSCREEN_NOCAPTION);
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

    if (!wxLaunchDefaultApplication(Log::getFileName()))
    {
        wxString msg;
        msg << "Failed to open log file '" << Log::getFileName() << "'.";
        Dialog::get().getConfirmation("Failed to open file", msg);
    }
    event.Skip();
}

void Window::onConfig(wxCommandEvent& event)
{
    static bool shown = false;
    if (!shown)
    {
        Dialog::get().getConfirmation(_("Attention"),
            _("Note that changing this file will not update the gui.\n"
            "Changed settings are applied only after restarting the application.\n"
            "Incorrectly changed settings may cause the application to crash upon startup.\n"
            "If that happens, delete the file from disk (or make the file empty) and restart.\n"));
        shown = true;
    }
    if (!wxLaunchDefaultApplication(Config::getFileName()))
    {
        wxString msg;
        msg << "Failed to open config file '" << Log::getFileName() << "'.";
        Dialog::get().getConfirmation("Failed to open file", msg);
    }
    event.Skip();
}

void Window::onAbout(wxCommandEvent& event)
{
    DialogAbout* dialog = new DialogAbout();
    dialog->ShowModal();
    event.Skip();
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
    wxMenu* previous = 0;
    bool enable = enabled;
    if (menu == 0)
    {
        menu = mMenuSequence;
        enable = false;
    }
    if (mMenuBar->GetMenu(sSequenceMenuIndex) != menu)
    {
        // Only in case of changes. Otherwise wxWidgets asserts.
        previous = mMenuBar->Replace(sSequenceMenuIndex, menu, _("&Sequence"));
    }
    mMenuBar->EnableTop(sSequenceMenuIndex,enable);
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

void Window::setAdditionalTitle(const wxString& title)
{
    SetTitle(sTitle + ": " + title);
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void Window::updateWorkspaceMenu()
{
    bool enable = ! Config::WorkspacePerspectives::get().empty();
    mMenuWorkspace->Enable(ID_WORKSPACE_LOAD, enable);
    mMenuWorkspace->Enable(ID_WORKSPACE_DELETE, enable);
    mMenuWorkspace->Enable(ID_WORKSPACE_DELETEALL, enable);
}

void Window::updateTitle()
{
    if (GetDocumentManager()->GetCurrentDocument() != 0)
    {
        SetTitle(model::Project::get().getRoot()->getName() + " - " + sTitle);
    }
    else
    {
        SetTitle(sTitle);
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
    catch (boost::archive::archive_exception& e) { VAR_ERROR(e.what());                         throw; }
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
        catch (boost::archive::archive_exception& e) { VAR_ERROR(e.what());                         throw; }
        catch (boost::exception &e)                  { VAR_ERROR(boost::diagnostic_information(e)); throw; }
        catch (std::exception& e)                    { VAR_ERROR(e.what());                         throw; }
        catch (...)                                  { LOG_ERROR;                                   throw; }
    }
    template void IView::serialize<boost::archive::xml_oarchive>(boost::archive::xml_oarchive& ar, const unsigned int archiveVersion);
    template void IView::serialize<boost::archive::xml_iarchive>(boost::archive::xml_iarchive& ar, const unsigned int archiveVersion);
}