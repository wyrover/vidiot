#include "Window.h"

#include "AboutDialog.h"
#include "AudioTransitionFactory.h"
#include "CommandLine.h"
#include "Config.h"
#include "Dialog.h"
#include "ids.h"
#include "Layout.h"
#include "Node.h"
#include "Options.h"
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
    ,   menubar(0)
    ,   menuedit(0)
    ,   menusequence(0)
    ,   mTestCrash(new util::TestCrash(this))
    ,   mAudioTransitionFactory(new model::audio::AudioTransitionFactory())
    ,   mVideoTransitionFactory(new model::video::VideoTransitionFactory())
{
    ::wxInitAllImageHandlers();

    // Construction not done in constructor list due to dependency on sCurrent
    mWorker         = new Worker();
    mWatcher        = new Watcher();
    mPreview        = new Preview(this); // Must be opened before timelinesview for the case of autoloading with open sequences/timelines
    mDetailsView    = new DetailsView(this);
    mTimelinesView  = new TimelinesView(this);
    mProjectView    = new ProjectView(this);

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
    menufile->Append(wxID_EXIT, _("E&xit"));

    menuedit = new wxMenu();
    menuedit->Append(wxID_UNDO);
    menuedit->Append(wxID_REDO);
    menuedit->AppendSeparator();
    menuedit->Append(wxID_CUT);
    menuedit->Append(wxID_COPY);
    menuedit->Append(wxID_PASTE);

    wxMenu* menuview = new wxMenu();
    menuview->AppendCheckItem(ID_SNAP_CLIPS, _("Snap to clips"));
    menuview->Check(ID_SNAP_CLIPS, Config::ReadBool(Config::sPathSnapClips));
    menuview->AppendCheckItem(ID_SNAP_CURSOR, _("Snap to cursor"));
    menuview->Check(ID_SNAP_CURSOR, Config::ReadBool(Config::sPathSnapCursor));
    menuview->AppendSeparator();
    menuview->AppendCheckItem(ID_SHOW_BOUNDINGBOX, _("Show bounding box"));
    menuview->Check(ID_SHOW_BOUNDINGBOX, Config::ReadBool(Config::sPathShowBoundingBox));

    menusequence = new wxMenu();

    wxMenu* menutools = new wxMenu();
    menutools->Append(wxID_PREFERENCES, _("&Options"));

    wxMenu* menuhelp = new wxMenu();
    menuhelp->Append(wxID_HELP, _("Help"));
    menuhelp->AppendSeparator();
    menuhelp->Append(ID_OPENLOGFILE, _("Open log file"));
    menuhelp->AppendSeparator();
    menuhelp->Append(wxID_ABOUT, _("&About..."));

    menubar = new wxMenuBar();
    menubar->Append(menufile,     _("&File"));
    menubar->Append(menuedit,     _("&Edit"));
    menubar->Append(menuview,     _("&View"));
    menubar->Append(menusequence, _("&Sequence"));
    sSequenceMenuIndex = 3;
    menubar->Append(menutools,    _("&Tools"));
    menubar->Append(mTestCrash->getMenu(),     _("&Crash"));
    menubar->Append(menuhelp,     _("&Help"));

    SetMenuBar( menubar );

    menubar->EnableTop(sSequenceMenuIndex,false); // Disable sequence menu

    SetStatusBar(new StatusBar(this));

    mUiManager.SetManagedWindow(this);
    mUiManager.InsertPane(mProjectView,     wxAuiPaneInfo().BestSize(wxSize(100,300)).MinSize(wxSize(100,300)).Top().Position(0).CaptionVisible(false));
    mUiManager.InsertPane(mDetailsView,     wxAuiPaneInfo().Top().Position(1).Resizable().CaptionVisible(false));
    mUiManager.InsertPane(mPreview,         wxAuiPaneInfo().BestSize(wxSize(100,300)).MinSize(wxSize(100,300)).Top().Position(2).CaptionVisible(false));
    mUiManager.InsertPane(mTimelinesView,   wxAuiPaneInfo().BestSize(wxSize(400,100)).MinSize(wxSize(400,100)).Center().CaptionVisible(false));
    mUiManager.SetFlags(wxAUI_MGR_LIVE_RESIZE);
    mUiManager.Update();

    Bind(model::EVENT_OPEN_PROJECT,     &Window::onOpenProject,     this);
    Bind(model::EVENT_CLOSE_PROJECT,    &Window::onCloseProject,    this);
    Bind(model::EVENT_RENAME_PROJECT,   &Window::onRenameProject,   this);

    Bind(wxEVT_COMMAND_MENU_SELECTED,   &wxDocManager::OnFileClose,     GetDocumentManager(), wxID_CLOSE);
    Bind(wxEVT_COMMAND_MENU_SELECTED,   &wxDocManager::OnFileCloseAll,	GetDocumentManager(), wxID_CLOSE_ALL);
    Bind(wxEVT_COMMAND_MENU_SELECTED,   &wxDocManager::OnFileNew,       GetDocumentManager(), wxID_NEW);
    Bind(wxEVT_COMMAND_MENU_SELECTED,   &wxDocManager::OnFileOpen,      GetDocumentManager(), wxID_OPEN);
    Bind(wxEVT_COMMAND_MENU_SELECTED,   &wxDocManager::OnFileRevert,    GetDocumentManager(), wxID_REVERT);
    Bind(wxEVT_COMMAND_MENU_SELECTED,   &wxDocManager::OnFileSave,      GetDocumentManager(), wxID_SAVE);
    Bind(wxEVT_COMMAND_MENU_SELECTED,   &wxDocManager::OnFileSaveAs,    GetDocumentManager(), wxID_SAVEAS);

    Bind(wxEVT_COMMAND_MENU_SELECTED,   &wxDocManager::OnUndo,          GetDocumentManager(), wxID_UNDO);
    Bind(wxEVT_COMMAND_MENU_SELECTED,   &wxDocManager::OnRedo,          GetDocumentManager(), wxID_REDO);

    Bind(wxEVT_COMMAND_MENU_SELECTED,   &Window::onExit,             this, wxID_EXIT);
    Bind(wxEVT_COMMAND_MENU_SELECTED,   &Window::onSnapClips,        this, ID_SNAP_CLIPS);
    Bind(wxEVT_COMMAND_MENU_SELECTED,   &Window::onSnapCursor,       this, ID_SNAP_CURSOR);
    Bind(wxEVT_COMMAND_MENU_SELECTED,   &Window::onShowBoundingBox,  this, ID_SHOW_BOUNDINGBOX);
    Bind(wxEVT_COMMAND_MENU_SELECTED,   &Window::onHelp,             this, wxID_HELP);
    Bind(wxEVT_COMMAND_MENU_SELECTED,   &Window::onLog,              this, ID_OPENLOGFILE);
    Bind(wxEVT_COMMAND_MENU_SELECTED,   &Window::onAbout,            this, wxID_ABOUT);
    Bind(wxEVT_COMMAND_MENU_SELECTED,   &Window::onOptions,          this, wxID_PREFERENCES);

    GetDocumentManager()->SetMaxDocsOpen(1);
    GetDocumentManager()->FileHistoryUseMenu(menufile);
    GetDocumentManager()->FileHistoryLoad(*wxConfigBase::Get());

    if (Config::ReadBool(Config::sPathTest))
    {
        wxSize screenSize = wxGetDisplaySize();
        wxSize winSize = GetSize();
        Move(screenSize.GetWidth() - winSize.GetWidth(),0);
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

    Unbind(wxEVT_COMMAND_MENU_SELECTED,   &wxDocManager::OnFileClose,     GetDocumentManager(), wxID_CLOSE);
    Unbind(wxEVT_COMMAND_MENU_SELECTED,   &wxDocManager::OnFileCloseAll,  GetDocumentManager(), wxID_CLOSE_ALL);
    Unbind(wxEVT_COMMAND_MENU_SELECTED,   &wxDocManager::OnFileNew,       GetDocumentManager(), wxID_NEW);
    Unbind(wxEVT_COMMAND_MENU_SELECTED,   &wxDocManager::OnFileOpen,      GetDocumentManager(), wxID_OPEN);
    Unbind(wxEVT_COMMAND_MENU_SELECTED,   &wxDocManager::OnFileRevert,    GetDocumentManager(), wxID_REVERT);
    Unbind(wxEVT_COMMAND_MENU_SELECTED,   &wxDocManager::OnFileSave,      GetDocumentManager(), wxID_SAVE);
    Unbind(wxEVT_COMMAND_MENU_SELECTED,   &wxDocManager::OnFileSaveAs,    GetDocumentManager(), wxID_SAVEAS);

    Unbind(wxEVT_COMMAND_MENU_SELECTED,   &wxDocManager::OnUndo,          GetDocumentManager(), wxID_UNDO);
    Unbind(wxEVT_COMMAND_MENU_SELECTED,   &wxDocManager::OnRedo,          GetDocumentManager(), wxID_REDO);

    Unbind(wxEVT_COMMAND_MENU_SELECTED,   &Window::onExit,             this, wxID_EXIT);
    Unbind(wxEVT_COMMAND_MENU_SELECTED,   &Window::onSnapClips,        this, ID_SNAP_CLIPS);
    Unbind(wxEVT_COMMAND_MENU_SELECTED,   &Window::onSnapCursor,       this, ID_SNAP_CURSOR);
    Unbind(wxEVT_COMMAND_MENU_SELECTED,   &Window::onShowBoundingBox,  this, ID_SHOW_BOUNDINGBOX);

    Unbind(wxEVT_COMMAND_MENU_SELECTED,   &Window::onHelp,             this, wxID_HELP);
    Unbind(wxEVT_COMMAND_MENU_SELECTED,   &Window::onLog,              this, ID_OPENLOGFILE);
    Unbind(wxEVT_COMMAND_MENU_SELECTED,   &Window::onAbout,            this, wxID_ABOUT);
    Unbind(wxEVT_COMMAND_MENU_SELECTED,   &Window::onOptions,          this, wxID_PREFERENCES);

    mUiManager.UnInit();

    wxStatusBar* sb = GetStatusBar();
    SetStatusBar(0);
    sb->wxWindowBase::Destroy();

    delete mAudioTransitionFactory;
    delete mVideoTransitionFactory;

    delete mTestCrash; // Unbind the crash method
    setSequenceMenu(0); // Ensure destruction of sequenceMenu
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
    GetDocumentManager()->GetCurrentDocument()->GetCommandProcessor()->SetEditMenu(menuedit); // Set menu for do/undo
    GetDocumentManager()->GetCurrentDocument()->GetCommandProcessor()->Initialize();
    GetDocumentManager()->AddFileToHistory(model::Project::get().GetFilename());
    GetDocumentManager()->FileHistorySave(*wxConfigBase::Get());
    wxConfigBase::Get()->Flush();
    event.Skip();
}

void Window::onCloseProject( model::EventCloseProject &event )
{
    event.Skip();
}

void Window::onRenameProject( model::EventRenameProject &event )
{
    GetDocumentManager()->AddFileToHistory(model::Project::get().GetFilename());
    GetDocumentManager()->FileHistorySave(*wxConfigBase::Get());
    wxConfigBase::Get()->Flush();
    event.Skip();
}

//////////////////////////////////////////////////////////////////////////
// FILE MENU
//////////////////////////////////////////////////////////////////////////

void Window::onExit(wxCommandEvent &)
{
    Close();
}

//////////////////////////////////////////////////////////////////////////
// VIEW MENU
//////////////////////////////////////////////////////////////////////////

void Window::onSnapClips(wxCommandEvent& event)
{
    Config::WriteBool(Config::sPathSnapClips, event.IsChecked());
}

void Window::onSnapCursor(wxCommandEvent& event)
{
    Config::WriteBool(Config::sPathSnapCursor, event.IsChecked());
}

void Window::onShowBoundingBox(wxCommandEvent& event)
{
    Config::WriteBool(Config::sPathShowBoundingBox, event.IsChecked());
}

//////////////////////////////////////////////////////////////////////////
// TOOLS MENU
//////////////////////////////////////////////////////////////////////////

void Window::onOptions(wxCommandEvent& event)
{
    Options w(this);
    w.ShowModal();
}

//////////////////////////////////////////////////////////////////////////
// HELP MENU
//////////////////////////////////////////////////////////////////////////

void Window::onHelp(wxCommandEvent& event)
{
    wxMessageBox(_("No help yet..."), _("Help"), wxOK | wxICON_INFORMATION, this);
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

void Window::onAbout(wxCommandEvent& event)
{
    AboutDialog* dialog = new AboutDialog();
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

void Window::triggerLayout()
{
    mUiManager.Update();
}

//////////////////////////////////////////////////////////////////////////
// ENABLING/DISABLING MENUS
//////////////////////////////////////////////////////////////////////////

void Window::setSequenceMenu(wxMenu* menu)
{
    wxMenu* previous = 0;
    bool enable = true;
    if (menu == 0)
    {
        menu = menusequence;
        enable = false;
    }
    if (menubar->GetMenu(sSequenceMenuIndex) != menu)
    {
        // Only in case of changes. Otherwise wxWidgets asserts.
        previous = menubar->Replace(sSequenceMenuIndex, menu, _("&Sequence"));
        menubar->EnableTop(sSequenceMenuIndex,enable);
    }
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

void Window::setAdditionalTitle(wxString title)
{
    SetTitle(sTitle + ": " + title);
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void Window::serialize(Archive & ar, const unsigned int version)
{
    ar & *mProjectView;
    ar & *mTimelinesView;
    ar & *mWatcher;
}
template void Window::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void Window::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);
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
        ar & static_cast<gui::Window&>(*this);
    }
    template void IView::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
    template void IView::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);
}