#include "GuiWindow.h"

#include <wx/docview.h>
#include <wx/confbase.h>
#include <wx/msgdlg.h>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include "GuiMain.h"
#include "GuiOptions.h"
#include "GuiView.h"
#include "GuiProjectView.h"
#include "GuiPreview.h"
#include "GuiTimelinesView.h"
#include "AProjectViewNode.h"
#include "UtilLog.h"

namespace gui {

enum {
    meID_ADDVIDEOTRACK = wxID_HIGHEST+1,
    meID_ADDAUDIOTRACK,
    meID_CLOSESEQUENCE,
    meID_PLAYSEQUENCE,
    meID_OPTIONS
};

GuiWindow::GuiWindow()
:   wxDocParentFrame()
,	mDocManager()
,	mDocTemplate(0)
{
    // Must be done in two step construction way, since it reuses mDocManger which would
    // be initialized last if the initialization of the base class was also done in the
    // constructor list.
    wxDocParentFrame::Create(&mDocManager, 0, wxID_ANY, _("Vidiot"), wxDefaultPosition, wxSize(1200,800));
    mDocTemplate = new wxDocTemplate(&mDocManager, _("Vidiot files"), "*.vid", "", "vid", _("Vidiot Project"), _("Vidiot Project View"), CLASSINFO(model::Project), CLASSINFO(GuiView));

    mTimelinesView  = new GuiTimelinesView(this);
    mPreview        = new GuiPreview(this); // Must be opened before timelinesview for the case of autoloading with open sequences/timelines
    mProjectView    = new GuiProjectView(this);
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

    wxMenu* menusequence = new wxMenu();
    menusequence->Append(meID_PLAYSEQUENCE, _("Play"));
    menusequence->AppendSeparator();
    menusequence->Append(meID_ADDVIDEOTRACK, _("Add video track"));
    menusequence->Append(meID_ADDAUDIOTRACK, _("Add audio track"));
    menusequence->AppendSeparator();
    menusequence->Append(meID_CLOSESEQUENCE, _("Close"));

    wxMenu* menutools = new wxMenu();
    menutools->Append(meID_OPTIONS, _("&Options"));

    wxMenu* menuhelp = new wxMenu();
    menuhelp->Append(wxID_HELP, _("Help"));

    menusequence->AppendSeparator();
    menuhelp->Append(wxID_INFO, _("Dump"));
    menusequence->AppendSeparator();
    menuhelp->Append(wxID_ABOUT, _("&About..."));

    menubar = new wxMenuBar();
    menubar->Append(menufile,     _("&File"));
    menubar->Append(menuedit,     _("&Edit"));
    menubar->Append(menusequence, _("&Sequence"));
    menubar->Append(menutools,    _("&Tools"));
    menubar->Append(menuhelp,     _("&Help"));

    SetMenuBar( menubar );

    menubar->EnableTop(2,false); // Disable sequence menu

    CreateStatusBar(4);
    SetStatusText( _T("Welcome to wxWindows!"), 0 );
    SetStatusText( _T("Status1"), 1 );
    SetStatusText( _T("Status2"), 2 );
    SetProcessingText(_(""));

    mUiManager.SetManagedWindow(this);
    mUiManager.InsertPane(mProjectView,     wxAuiPaneInfo().BestSize(wxSize(200,500)).MinSize(wxSize(200,500)).Top().Position(0).CaptionVisible(false));
    mUiManager.InsertPane(mEditor,          wxAuiPaneInfo().BestSize(wxSize(200,100)).MinSize(wxSize(200,100)).Top().Position(1).CaptionVisible(false));
    mUiManager.InsertPane(mPreview,         wxAuiPaneInfo().BestSize(wxSize(200,100)).MinSize(wxSize(400,100)).Top().Position(2).CaptionVisible(false));
    mUiManager.InsertPane(mTimelinesView,   wxAuiPaneInfo().BestSize(wxSize(200,100)).MinSize(wxSize(200,100)).Center().CaptionVisible(false));
    mUiManager.SetFlags(wxAUI_MGR_LIVE_RESIZE);
    mUiManager.Update();

    wxGetApp().Bind(model::EVENT_OPEN_PROJECT,     &GuiWindow::OnOpenProject,              this);
    wxGetApp().Bind(model::EVENT_CLOSE_PROJECT,    &GuiWindow::OnCloseProject,             this);

    Bind(wxEVT_COMMAND_MENU_SELECTED,   &wxDocManager::OnFileClose,     &mDocManager, wxID_CLOSE);
    Bind(wxEVT_COMMAND_MENU_SELECTED,   &wxDocManager::OnFileCloseAll,	&mDocManager, wxID_CLOSE_ALL);
    Bind(wxEVT_COMMAND_MENU_SELECTED,   &wxDocManager::OnFileNew,       &mDocManager, wxID_NEW);
    Bind(wxEVT_COMMAND_MENU_SELECTED,   &wxDocManager::OnFileOpen,      &mDocManager, wxID_OPEN);
    Bind(wxEVT_COMMAND_MENU_SELECTED,   &wxDocManager::OnFileRevert,    &mDocManager, wxID_REVERT);
    Bind(wxEVT_COMMAND_MENU_SELECTED,   &wxDocManager::OnFileSave,      &mDocManager, wxID_SAVE);
    Bind(wxEVT_COMMAND_MENU_SELECTED,   &wxDocManager::OnFileSaveAs,    &mDocManager, wxID_SAVEAS);

    Bind(wxEVT_COMMAND_MENU_SELECTED,   &wxDocManager::OnUndo,          &mDocManager, wxID_UNDO);
    Bind(wxEVT_COMMAND_MENU_SELECTED,   &wxDocManager::OnRedo,          &mDocManager, wxID_REDO);

    
    Bind(wxEVT_COMMAND_MENU_SELECTED,   &GuiWindow::OnExit,             this, wxID_EXIT);
    Bind(wxEVT_COMMAND_MENU_SELECTED,   &GuiWindow::OnPlaySequence,     this, meID_PLAYSEQUENCE);
    Bind(wxEVT_COMMAND_MENU_SELECTED,   &GuiWindow::OnCloseSequence,    this, meID_CLOSESEQUENCE);
    Bind(wxEVT_COMMAND_MENU_SELECTED,   &GuiWindow::OnHelp,             this, wxID_HELP);
    Bind(wxEVT_COMMAND_MENU_SELECTED,   &GuiWindow::OnAbout,            this, wxID_ABOUT);
    Bind(wxEVT_COMMAND_MENU_SELECTED,   &GuiWindow::OnAddVideoTrack,    this, meID_ADDVIDEOTRACK);
    Bind(wxEVT_COMMAND_MENU_SELECTED,   &GuiWindow::OnAddAudioTrack,    this, meID_ADDAUDIOTRACK);
    Bind(wxEVT_COMMAND_MENU_SELECTED,   &GuiWindow::OnOptions,          this, meID_OPTIONS);

    Bind(wxEVT_CLOSE_WINDOW,            &GuiWindow::OnCloseWindow,      this);

    mDocManager.SetMaxDocsOpen(1);
    mDocManager.FileHistoryUseMenu(menufile);
    mDocManager.FileHistoryLoad(*wxConfigBase::Get());

    Show();
}

void GuiWindow::init()
{
    if (GuiOptions::GetAutoLoad())
    {
        mDocManager.CreateDocument(*GuiOptions::GetAutoLoad(), wxDOC_SILENT);
    }
}

GuiWindow::~GuiWindow()
{
    mUiManager.UnInit();

    delete mDocTemplate;
    delete mTimelinesView;
    delete mPreview;
    delete mProjectView;
    delete mEditor;
}

//////////////////////////////////////////////////////////////////////////
// PROJECT EVENTS
//////////////////////////////////////////////////////////////////////////

void GuiWindow::OnOpenProject( model::EventOpenProject &event )
{
    GetDocumentManager()->GetCurrentDocument()->GetCommandProcessor()->SetEditMenu(menuedit); // Set menu for do/undo
    GetDocumentManager()->GetCurrentDocument()->GetCommandProcessor()->Initialize();
    mDocManager.FileHistorySave(*wxConfigBase::Get());
    GuiOptions::SetAutoLoadFilename(model::Project::current()->GetFilename());
    wxConfigBase::Get()->Flush();
    event.Skip();
}

void GuiWindow::OnCloseProject( model::EventCloseProject &event )
{
    GuiOptions::SetAutoLoadFilename("");
    event.Skip();
}

//////////////////////////////////////////////////////////////////////////
// GUI EVENTS
//////////////////////////////////////////////////////////////////////////

void GuiWindow::OnCloseWindow(wxCloseEvent& event)
{
    wxFrame::OnCloseWindow(event);
}

void GuiWindow::SetProcessingText(wxString text)
{
    SetStatusText( text, 3 );
}

//////////////////////////////////////////////////////////////////////////
// FILE MENU
//////////////////////////////////////////////////////////////////////////

void GuiWindow::OnExit(wxCommandEvent &)
{ 
    Close(); 
}

//////////////////////////////////////////////////////////////////////////
// SEQUENCE MENU
//////////////////////////////////////////////////////////////////////////

void GuiWindow::OnPlaySequence(wxCommandEvent& WXUNUSED(event))
{
    LOG_INFO;
    mPreview->play();
}

void GuiWindow::OnAddVideoTrack(wxCommandEvent& WXUNUSED(event))
{
    LOG_DEBUG;
    // todo handle this via timelinesview. that class is resp for maintaining the lst of sequences.
    //mProject->Submit(new command::TimelineCreateVideoTrack(*mOpenSequences.begin()));
}

void GuiWindow::OnAddAudioTrack(wxCommandEvent& WXUNUSED(event))
{
    LOG_DEBUG;
    // todo handle this via timelinesview. that class is resp for maintaining the lst of sequences.
//    mProject->Submit(new command::TimelineCreateAudioTrack(*mOpenSequences.begin()));
}

void GuiWindow::OnCloseSequence(wxCommandEvent& WXUNUSED(event))
{
    mTimelinesView->Close();
}

//////////////////////////////////////////////////////////////////////////
// TOOLS MENU
//////////////////////////////////////////////////////////////////////////

void GuiWindow::OnOptions(wxCommandEvent& WXUNUSED(event))
{
    GuiOptions w(this);
    w.ShowModal();
}

//////////////////////////////////////////////////////////////////////////
// HELP MENU
//////////////////////////////////////////////////////////////////////////

void GuiWindow::OnHelp(wxCommandEvent& WXUNUSED(event))
{
    NIY
}

void GuiWindow::OnAbout(wxCommandEvent& WXUNUSED(event))
{
    wxMessageBox(_("Vidiot 0.1"), _T("About"), wxOK | wxICON_INFORMATION, this);
}

//////////////////////////////////////////////////////////////////////////
// GET WIDGETS
//////////////////////////////////////////////////////////////////////////

GuiTimelinesView& GuiWindow::getTimeLines()
{
    return *mTimelinesView;
}

GuiPreview& GuiWindow::getPreview()
{
    return *mPreview;
}

GuiProjectView&	GuiWindow::getProjectView()
{
    return *mProjectView;
}

//////////////////////////////////////////////////////////////////////////
// ENABLING/DISABLING MENUS
//////////////////////////////////////////////////////////////////////////

void GuiWindow::EnableSequenceMenu(bool enable)
{
    menubar->EnableTop(2,enable);
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
