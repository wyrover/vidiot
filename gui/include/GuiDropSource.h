#ifndef GUI_DROP_SOURCE_H
#define GUI_DROP_SOURCE_H

#include <wx/dnd.h>
#include "GuiDataObject.h"
#include "AProjectViewNode.h"

class GuiDropSource : public wxDropSource
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION METHODS
    //////////////////////////////////////////////////////////////////////////

    GuiDropSource(wxWindow* win, model::ProjectViewPtrs assets);
    ~GuiDropSource();

    //////////////////////////////////////////////////////////////////////////
    // FROM WXDROPSOURCE
    //////////////////////////////////////////////////////////////////////////

    wxDragResult DoDragDrop(int flags=wxDrag_CopyOnly);
    bool GiveFeedback(wxDragResult effect);

private:

    GuiDataObject mDataObject;
    model::ProjectViewPtrs mAssets;
    wxWindow* mWin;
    wxFrame* mHint;
};


#endif // GUI_DROP_SOURCE_H