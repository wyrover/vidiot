#ifndef GUI_TIME_LINE_DROP_TARGET_H
#define GUI_TIME_LINE_DROP_TARGET_H

#include <wx/dnd.h>
#include "GuiDataObject.h"
#include "AProjectViewNode.h"
#include "GuiPtr.h"

namespace gui { namespace timeline {

class GuiTimeLineDropTarget : public wxDropTarget
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION METHODS
    //////////////////////////////////////////////////////////////////////////

    GuiTimeLineDropTarget(GuiTimeLineZoomPtr zoom, GuiTimeLinePtr timeline);
    ~GuiTimeLineDropTarget();

    //////////////////////////////////////////////////////////////////////////
    // ACCESSORS
    //////////////////////////////////////////////////////////////////////////

    bool isDragging() const;

    //////////////////////////////////////////////////////////////////////////
    // FROM WXDROPTARGET
    //////////////////////////////////////////////////////////////////////////

    virtual bool GetData();
    wxDragResult OnData (wxCoord x, wxCoord y, wxDragResult def);
    wxDragResult OnDragOver (wxCoord x, wxCoord y, wxDragResult def);
    bool OnDrop (wxCoord x, wxCoord y);
    wxDragResult OnEnter (wxCoord x, wxCoord y, wxDragResult def);
    void OnLeave ();

private:

    GuiTimeLineZoomPtr mZoom;
    GuiTimeLinePtr mTimeLine;
};

}} // namespace

#endif // GUI_TIME_LINE_DROP_TARGET_H