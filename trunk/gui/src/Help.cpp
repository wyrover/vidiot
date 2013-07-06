#include "Help.h"

#include "Config.h"
#include "UtilLog.h"
#include "Window.h"
#include "UtilLogWxwidgets.h"

namespace gui {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

Help::Help(wxWindow* parent)
    :   wxPanel(parent,wxID_ANY,wxDefaultPosition,wxDefaultSize )
    ,   mBack(0)
{
    VAR_DEBUG(this);

    SetSizer(new wxBoxSizer(wxVERTICAL));

    wxString dir = wxFileName(Config::getExeDir() + "\\html\\help\\","").GetFullPath();

    ////////  ////////

    mHtml = new wxHtmlWindow(this);
    mHtml->LoadPage(wxFileName(dir, "index.html").GetFullPath()); // todo remove this class

    ////////  ////////

    GetSizer()->Add(mHtml, wxSizerFlags(1).Expand());

    ////////  ////////

    wxSizer* buttons = new wxBoxSizer(wxHORIZONTAL);
    ASSERT_NONZERO(buttons);
    mBack = new wxButton(this,wxID_ANY,_("Back"));
    buttons->Add(mBack);
    GetSizer()->Add(buttons);

    mBack->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &Help::onBack, this);

    //// Position dialog alongside the main window
    //wxSize mainWindowSize = Window::get().GetSize();
    //wxPoint mainWindowPosition = Window::get().GetPosition();
    //wxSize helpWindowSize = GetSize();

    //int screenWidth, screenHeight;
    //wxDisplaySize(&screenWidth,&screenHeight);

    //if (mainWindowPosition.x + mainWindowSize.GetWidth() + helpWindowSize.x > screenWidth)
    //{
    //    // Won't fit if we just position the dialog alongside.

    //    if (mainWindowSize.GetWidth() + helpWindowSize.x > screenWidth)
    //    {
    //        // Main window must be resized, otherwise it'll never fit
    //        Window::get().SetSize(screenWidth - helpWindowSize.x,  mainWindowSize.y);
    //    }

    //    // Now reposition both windows if required
    //    else
    //    {
    //        // Just moving the main window a bit will do the trick
    //    }
    //}
}

Help::~Help()
{

    VAR_DEBUG(this);
    mBack->Unbind(wxEVT_COMMAND_BUTTON_CLICKED, &Help::onBack, this);
}

void Help::onBack(wxCommandEvent &event)
{
    mHtml->HistoryBack();
    event.Skip();
}

} //namespace