#include "GuiView.h"
#include "wxInclude.h"
#include "UtilLog.h"
#include "Project.h"
#include "GuiMain.h"
#include "GuiWindow.h"

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

GuiView::GuiView()
:   wxView()
{
}

GuiView::~GuiView()
{
}

//////////////////////////////////////////////////////////////////////////
// GUI METHODS
//////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC_CLASS(GuiView, wxView)

bool GuiView::OnCreate(wxDocument *doc, long WXUNUSED(flags) )
{
    Activate(true); // Make sure the document manager knows that this is the current view.
    return true;
}

void GuiView::OnDraw(wxDC *dc)
{
}

void GuiView::OnUpdate(wxView *sender, wxObject *WXUNUSED(hint))
{
}

bool GuiView::OnClose(bool deleteWindow)
{
    Activate(false);
    return true;
}
