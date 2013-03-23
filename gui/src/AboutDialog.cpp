#include "AboutDialog.h"

#include "UtilLog.h"
#include "Window.h"
#include "UtilLogWxwidgets.h"
#include <wx/html/htmlwin.h>

namespace gui {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

//void addOption(wxWindow* parent, wxSizer* vSizer, wxString name, wxWindow* option)
//{
//    wxStaticText* wxst = new wxStaticText(parent,wxID_ANY,name,wxDefaultPosition,wxDefaultSize,wxST_ELLIPSIZE_MIDDLE);
//    wxst->SetMinSize(wxSize(120,-1));
//    wxst->SetSize(wxSize(120,-1));
//    wxBoxSizer* hSizer = new wxBoxSizer(wxHORIZONTAL);
//    hSizer->Add(wxst,wxSizerFlags(1));
//    hSizer->Add(option,wxSizerFlags(2).Right());
//    vSizer->Add(hSizer,wxSizerFlags().Expand());
//};

AboutDialog::AboutDialog()
    :   wxDialog(&Window::get(),wxID_ANY,_("About"),wxDefaultPosition,wxSize(600,600),wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER,wxDialogNameStr )
{
    VAR_DEBUG(this);

    SetSizer(new wxBoxSizer(wxVERTICAL));

    wxNotebook* book = new wxNotebook(this,wxID_ANY);

    wxString exepath;
    wxFileName::SplitPath(wxStandardPaths::Get().GetExecutablePath(),&exepath,0,0);
    wxFileName dir = wxFileName(exepath,"");
    ASSERT(!dir.HasExt());
    ASSERT(!dir.HasName());
    dir.AppendDir("html");
    dir.AppendDir("about");

    ////////  ////////

    wxHtmlWindow* main = new wxHtmlWindow(book);
    VAR_ERROR(dir)(dir.GetFullPath());
    main->LoadPage(wxFileName(dir.GetFullPath(), "main.html").GetFullPath());
    book->AddPage(main, _("Vidiot"), true);

    // todo test for testauto+manual (cmake stuff)

    ////////  ////////

    wxHtmlWindow* wxwidgets = new wxHtmlWindow(book);
    //wxwidgets->LoadFile(wxFileName(dir.GetFullPath(), "wxwidgets.html"));
    book->AddPage(wxwidgets, _("wxWidgets"));

    ////////  ////////

    GetSizer()->Add(book, wxSizerFlags(1).Expand());

    ////////  ////////

    wxSizer* buttons = CreateButtonSizer(wxOK);
    ASSERT_NONZERO(buttons);
    GetSizer()->Add(buttons);
//    Fit();
}

AboutDialog::~AboutDialog()
{

    VAR_DEBUG(this);
}

} //namespace