#ifndef GUI_VIEW_H
#define GUI_VIEW_H

#include "wx/docview.h"

class GuiView
:   public wxView
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    GuiView();
    virtual ~GuiView();

    //////////////////////////////////////////////////////////////////////////
    // wxView
    //////////////////////////////////////////////////////////////////////////

    void OnDraw(wxDC *dc);
    bool OnCreate(wxDocument *doc, long flags);
    void OnUpdate(wxView *sender, wxObject *hint = NULL);
    bool OnClose(bool deleteWindow = true);

    DECLARE_DYNAMIC_CLASS(GuiView)
};

#endif // GUI_VIEW_H