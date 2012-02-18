#include "Window.h"

#include <wx/docview.h>
#include <wx/gdicmn.h>
#include <wx/msgdlg.h>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include "Dialog.h"
#include "Node.h"
#include "Config.h"
#include "ids.h"
#include "Options.h"
#include "Preview.h"
#include "Project.h"
#include "ProjectEvent.h"
#include "ProjectView.h"
#include "TimelinesView.h"
#include "UtilTestCrash.h"
#include "UtilLog.h"
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
static Window* sCurrent = 0;
static const wxString sTitle(_("Vidiot"));

Window::Window()
    :   wxDocParentFrame(new wxDocManager(), 0, wxID_ANY, sTitle, wxDefaultPosition, wxSize(1000,600))
    ,	mDocTemplate(new wxDocTemplate(GetDocumentManager(), _("Vidiot files"), "*.vid", "", "vid", _("Vidiot Project"), _("Vidiot Project View"), CLASSINFO(model::Project), CLASSINFO(ViewHelper)))
    ,   mDialog(new Dialog())
    ,   mWatcher(0)
    ,   mWorker(0)
    ,   mPreview(0)
    ,   mTimelinesView(0)
    ,   mProjectView(0)
    ,   menubar(0)
    ,   menuedit(0)
    ,   menusequence(0)
    ,   mTestCrash(new util::TestCrash(this))
{
    sCurrent = this;

    // Construction not done in constructor list due to dependency on sCurrent
    mWorker         = new Worker();
    mWatcher        = new Watcher();
    mPreview        = new Preview(this); // Must be opened before timelinesview for the case of autoloading with open sequences/timelines
    mTimelinesView  = new TimelinesView(this);
    mProjectView    = new ProjectView(this);

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

    menusequence = new wxMenu();

    wxMenu* menutools = new wxMenu();
    menutools->Append(ID_OPTIONS, _("&Options"));

    wxMenu* menuhelp = new wxMenu();
    menuhelp->Append(wxID_HELP, _("Help"));
    menuhelp->AppendSeparator();
    menuhelp->Append(wxID_INFO, _("Dump"));
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

    CreateStatusBar(getNumberOfStatusBars());
    setDebugText(_(""));
    setProcessingText(_(""));

    mUiManager.SetManagedWindow(this);
    mUiManager.InsertPane(mProjectView,     wxAuiPaneInfo().BestSize(wxSize(100,300)).MinSize(wxSize(100,300)).Top().Position(0).CaptionVisible(false));
    mUiManager.InsertPane(mPreview,         wxAuiPaneInfo().BestSize(wxSize(100,300)).MinSize(wxSize(100,300)).Top().Position(1).CaptionVisible(false));
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
    Bind(wxEVT_COMMAND_MENU_SELECTED,   &Window::onHelp,             this, wxID_HELP);
    Bind(wxEVT_COMMAND_MENU_SELECTED,   &Window::onAbout,            this, wxID_ABOUT);
    Bind(wxEVT_COMMAND_MENU_SELECTED,   &Window::onOptions,          this, ID_OPTIONS);

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
}

void Window::init()
{
    if (wxConfigBase::Get()->ReadBool(Config::sPathAutoLoadEnabled,false))
    {
        wxFileHistory* history = GetDocumentManager()->GetFileHistory();
        if (history->GetCount() > 0)
        {
            GetDocumentManager()->CreateDocument(history->GetHistoryFile(0), wxDOC_SILENT);
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
    Unbind(wxEVT_COMMAND_MENU_SELECTED,   &Window::onHelp,             this, wxID_HELP);
    Unbind(wxEVT_COMMAND_MENU_SELECTED,   &Window::onAbout,            this, wxID_ABOUT);
    Unbind(wxEVT_COMMAND_MENU_SELECTED,   &Window::onOptions,          this, ID_OPTIONS);

    mUiManager.UnInit();

    delete mTestCrash; // Unbind the crash method
    setSequenceMenu(0); // Ensure destruction of sequenceMenu
    delete mProjectView;    // First, delete the referring windows.
    delete mTimelinesView;  // Fixed deletion order is required. ProjectView 'knows/uses' the timeline view,
    delete mPreview;        // the timeline view in turn 'knows/uses' the preview (specifically, the player).
    delete mWatcher;
    delete mWorker;
    delete mDialog;
    //NOT: delete mDocTemplate;

    sCurrent = 0;
}

// static
Window& Window::get()
{
    ASSERT(sCurrent);
    return *sCurrent;
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
// GUI EVENTS
//////////////////////////////////////////////////////////////////////////

int Window::getNumberOfStatusBars() const
{
    return (Config::getShowDebugInfo() ? 2 : 1);
}

void Window::setDebugText(wxString text)
{
    if (Config::getShowDebugInfo())
    {
        SetStatusText( text, 0 );
    }
}

void Window::setProcessingText(wxString text)
{
    SetStatusText( text, Config::getShowDebugInfo() ? 1 : 0 );
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
    wxConfigBase::Get()->Write(Config::sPathSnapClips, event.IsChecked());
    wxConfigBase::Get()->Flush();
}

void Window::onSnapCursor(wxCommandEvent& event)
{
    wxConfigBase::Get()->Write(Config::sPathSnapCursor, event.IsChecked());
    wxConfigBase::Get()->Flush();
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
    NIY;
}

void Window::onAbout(wxCommandEvent& event)
{
    wxMessageBox(_("Vidiot 0.1"), _T("About"), wxOK | wxICON_INFORMATION, this);

    event.Skip();
}

//////////////////////////////////////////////////////////////////////////
// GET WIDGETS
//////////////////////////////////////////////////////////////////////////

TimelinesView& Window::getTimeLines()
{
    return *mTimelinesView;
}

Preview& Window::getPreview()
{
    return *mPreview;
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
    IView& IView::get()
    {
        return *gui::sCurrent;
    }

    template<class Archive>
    void IView::serialize(Archive & ar, const unsigned int version)
    {
        ar & static_cast<gui::Window&>(*this);
    }
    template void IView::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
    template void IView::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);
}