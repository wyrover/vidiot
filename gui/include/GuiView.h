#ifndef GUI_VIEW_H
#define GUI_VIEW_H

#include "wx/docview.h"

namespace gui {

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

} // namespace

#endif // GUI_VIEW_H