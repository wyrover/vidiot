#include "GuiWindow.h"

#include <wx/docview.h>
#include <wx/confbase.h>
#include <wx/msgdlg.h>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include "GuiMain.h"
#include "GuiOptions.h"
#include "ProjectView.h"
#include "GuiPreview.h"
#include "GuiTimelinesView.h"
#include "FSWatcher.h"
#include "AProjectViewNode.h"
#include "UtilLog.h"
#include "ids.h"

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
    void OnDraw(wxDC *dc) {}
    void OnUpdate(wxView *sender, wxObject *hint = NULL) {}
    bool OnCreate(wxDocument *doc, long flags) 
    {
        Activate(true); // Make sure the document manager knows that this is the current view.
        return true;
    }
    bool OnClose(bool deleteWindow = true)
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

GuiWindow::GuiWindow()
    :   wxDocParentFrame()
    ,	mDocManager(new wxDocManager())
    ,	mDocTemplate(new wxDocTemplate(mDocManager, _("Vidiot files"), "*.vid", "", "vid", _("Vidiot Project"), _("Vidiot Project View"), CLASSINFO(model::Project), CLASSINFO(ViewHelper)))
    ,   mWatcher(new FSWatcher())
    ,   menubar(0)
    ,   menuedit(0)
    ,   menusequence(0)
{
    // Must be done in two step construction way, since it reuses mDocManager which would
    // be initialized last if the initialization of the base class was also done in the
    // constructor list.
    wxDocParentFrame::Create(mDocManager, 0, wxID_ANY, _("Vidiot"), wxDefaultPosition, wxSize(1200,800));

    mTimelinesView  = new GuiTimelinesView(this);
    mPreview        = new GuiPreview(this); // Must be opened before timelinesview for the case of autoloading with open sequences/timelines
    mProjectView    = new ProjectView(this);
    mEditor         = new wxPanel(this);

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
    menubar->Append(menusequence, _("&Sequence"));
    menubar->Append(menutools,    _("&Tools"));
    menubar->Append(menuhelp,     _("&Help"));

    SetMenuBar( menubar );

    menubar->EnableTop(2,false); // Disable sequence menu

    CreateStatusBar(9);
    SetStatusText( _T("LMB:"), 1 );
    SetStatusText( _T("MMB:"), 2 );
    SetStatusText( _T("RMB:"), 3 );
    SetStatusText( _T("WHEEL:"), 4 );
    SetStatusText( _T("CTRL:"), 5 );
    SetStatusText( _T("SHIFT:"), 6 );
    SetStatusText( _T("ALT:"), 7 );
    SetProcessingText(_(""));

    mUiManager.SetManagedWindow(this);
    mUiManager.InsertPane(mProjectView,     wxAuiPaneInfo().BestSize(wxSize(200,500)).MinSize(wxSize(200,500)).Top().Position(0).CaptionVisible(false));
    mUiManager.InsertPane(mEditor,          wxAuiPaneInfo().BestSize(wxSize(200,100)).MinSize(wxSize(200,100)).Top().Position(1).CaptionVisible(false));
    mUiManager.InsertPane(mPreview,         wxAuiPaneInfo().BestSize(wxSize(200,100)).MinSize(wxSize(400,100)).Top().Position(2).CaptionVisible(false));
    mUiManager.InsertPane(mTimelinesView,   wxAuiPaneInfo().BestSize(wxSize(200,100)).MinSize(wxSize(200,100)).Center().CaptionVisible(false));
    mUiManager.SetFlags(wxAUI_MGR_LIVE_RESIZE);
    mUiManager.Update();

    wxGetApp().Bind(model::EVENT_OPEN_PROJECT,     &GuiWindow::onOpenProject,              this);
    wxGetApp().Bind(model::EVENT_CLOSE_PROJECT,    &GuiWindow::onCloseProject,             this);

    Bind(wxEVT_COMMAND_MENU_SELECTED,   &wxDocManager::OnFileClose,     mDocManager, wxID_CLOSE);
    Bind(wxEVT_COMMAND_MENU_SELECTED,   &wxDocManager::OnFileCloseAll,	mDocManager, wxID_CLOSE_ALL);
    Bind(wxEVT_COMMAND_MENU_SELECTED,   &wxDocManager::OnFileNew,       mDocManager, wxID_NEW);
    Bind(wxEVT_COMMAND_MENU_SELECTED,   &wxDocManager::OnFileOpen,      mDocManager, wxID_OPEN);
    Bind(wxEVT_COMMAND_MENU_SELECTED,   &wxDocManager::OnFileRevert,    mDocManager, wxID_REVERT);
    Bind(wxEVT_COMMAND_MENU_SELECTED,   &wxDocManager::OnFileSave,      mDocManager, wxID_SAVE);
    Bind(wxEVT_COMMAND_MENU_SELECTED,   &wxDocManager::OnFileSaveAs,    mDocManager, wxID_SAVEAS);

    Bind(wxEVT_COMMAND_MENU_SELECTED,   &wxDocManager::OnUndo,          mDocManager, wxID_UNDO);
    Bind(wxEVT_COMMAND_MENU_SELECTED,   &wxDocManager::OnRedo,          mDocManager, wxID_REDO);

    Bind(wxEVT_COMMAND_MENU_SELECTED,   &GuiWindow::onExit,             this, wxID_EXIT);
    Bind(wxEVT_COMMAND_MENU_SELECTED,   &GuiWindow::onHelp,             this, wxID_HELP);
    Bind(wxEVT_COMMAND_MENU_SELECTED,   &GuiWindow::onAbout,            this, wxID_ABOUT);
    Bind(wxEVT_COMMAND_MENU_SELECTED,   &GuiWindow::onOptions,          this, ID_OPTIONS);

    mDocManager->SetMaxDocsOpen(1);
    mDocManager->FileHistoryUseMenu(menufile);
    mDocManager->FileHistoryLoad(*wxConfigBase::Get());

    Show();
}

void GuiWindow::init()
{
    if (GuiOptions::GetAutoLoad())
    {
        mDocManager->CreateDocument(*GuiOptions::GetAutoLoad(), wxDOC_SILENT);
    }
}

GuiWindow::~GuiWindow()
{
    wxGetApp().Unbind(model::EVENT_OPEN_PROJECT,     &GuiWindow::onOpenProject,              this);
    wxGetApp().Unbind(model::EVENT_CLOSE_PROJECT,    &GuiWindow::onCloseProject,             this);

    Unbind(wxEVT_COMMAND_MENU_SELECTED,   &wxDocManager::OnFileClose,     mDocManager, wxID_CLOSE);
    Unbind(wxEVT_COMMAND_MENU_SELECTED,   &wxDocManager::OnFileCloseAll,  mDocManager, wxID_CLOSE_ALL);
    Unbind(wxEVT_COMMAND_MENU_SELECTED,   &wxDocManager::OnFileNew,       mDocManager, wxID_NEW);
    Unbind(wxEVT_COMMAND_MENU_SELECTED,   &wxDocManager::OnFileOpen,      mDocManager, wxID_OPEN);
    Unbind(wxEVT_COMMAND_MENU_SELECTED,   &wxDocManager::OnFileRevert,    mDocManager, wxID_REVERT);
    Unbind(wxEVT_COMMAND_MENU_SELECTED,   &wxDocManager::OnFileSave,      mDocManager, wxID_SAVE);
    Unbind(wxEVT_COMMAND_MENU_SELECTED,   &wxDocManager::OnFileSaveAs,    mDocManager, wxID_SAVEAS);

    Unbind(wxEVT_COMMAND_MENU_SELECTED,   &wxDocManager::OnUndo,          mDocManager, wxID_UNDO);
    Unbind(wxEVT_COMMAND_MENU_SELECTED,   &wxDocManager::OnRedo,          mDocManager, wxID_REDO);

    Unbind(wxEVT_COMMAND_MENU_SELECTED,   &GuiWindow::onExit,             this, wxID_EXIT);
    Unbind(wxEVT_COMMAND_MENU_SELECTED,   &GuiWindow::onHelp,             this, wxID_HELP);
    Unbind(wxEVT_COMMAND_MENU_SELECTED,   &GuiWindow::onAbout,            this, wxID_ABOUT);
    Unbind(wxEVT_COMMAND_MENU_SELECTED,   &GuiWindow::onOptions,          this, ID_OPTIONS);

    mUiManager.UnInit();

    delete mEditor;
    delete mProjectView;
    delete mPreview;
    delete mTimelinesView;
    //NOT: delete mDocTemplate;
    delete mDocManager;
}

//////////////////////////////////////////////////////////////////////////
// PROJECT EVENTS
//////////////////////////////////////////////////////////////////////////

void GuiWindow::onOpenProject( model::EventOpenProject &event )
{
    GetDocumentManager()->GetCurrentDocument()->GetCommandProcessor()->SetEditMenu(menuedit); // Set menu for do/undo
    GetDocumentManager()->GetCurrentDocument()->GetCommandProcessor()->Initialize();
    mDocManager->FileHistorySave(*wxConfigBase::Get());
    GuiOptions::SetAutoLoadFilename(model::Project::current()->GetFilename());
    wxConfigBase::Get()->Flush();
    event.Skip();
}

void GuiWindow::onCloseProject( model::EventCloseProject &event )
{
    GuiOptions::SetAutoLoadFilename("");
    event.Skip();
}

//////////////////////////////////////////////////////////////////////////
// GUI EVENTS
//////////////////////////////////////////////////////////////////////////

void GuiWindow::SetProcessingText(wxString text)
{
    SetStatusText( text, 8 );
}

//////////////////////////////////////////////////////////////////////////
// FILE MENU
//////////////////////////////////////////////////////////////////////////

void GuiWindow::onExit(wxCommandEvent &)
{ 
    Close(); 
}

//////////////////////////////////////////////////////////////////////////
// TOOLS MENU
//////////////////////////////////////////////////////////////////////////

void GuiWindow::onOptions(wxCommandEvent& WXUNUSED(event))
{
    GuiOptions w(this);
    w.ShowModal();
}

//////////////////////////////////////////////////////////////////////////
// HELP MENU
//////////////////////////////////////////////////////////////////////////

void GuiWindow::onHelp(wxCommandEvent& WXUNUSED(event))
{
    NIY
}

void GuiWindow::onAbout(wxCommandEvent& WXUNUSED(event))
{
    wxMessageBox(_("Vidiot 0.1"), _T("About"), wxOK | wxICON_INFORMATION, this);
}

//////////////////////////////////////////////////////////////////////////
// GET WIDGETS
//////////////////////////////////////////////////////////////////////////

// static
GuiWindow* GuiWindow::get()
{
    return dynamic_cast<GuiWindow*>(wxGetApp().GetTopWindow());
}

GuiTimelinesView& GuiWindow::getTimeLines()
{
    return *mTimelinesView;
}

GuiPreview& GuiWindow::getPreview()
{
    return *mPreview;
}

//////////////////////////////////////////////////////////////////////////
// ENABLING/DISABLING MENUS
//////////////////////////////////////////////////////////////////////////

void GuiWindow::setSequenceMenu(wxMenu* menu)
{
    wxMenu* previous = 0;
    bool enable = true;
    if (menu == 0)
    {
        menu = menusequence;
        enable = false;
    }
    if (menubar->GetMenu(2) != menu)
    {
        // Only in case of changes. Otherwise wxWidgets asserts.
        previous = menubar->Replace(2, menu, _("&Sequence"));
        menubar->EnableTop(2,enable);
    }
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION 
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void GuiWindow::serialize(Archive & ar, const unsigned int version)
{
    ar & *mProjectView;
    ar & *mTimelinesView;
}
template void GuiWindow::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void GuiWindow::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);

} // namespace
