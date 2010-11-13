#ifndef DROP_H
#define DROP_H

#include <wx/dnd.h>
#include "Part.h"

namespace gui { namespace timeline {

class Drop
    :   public Part
    ,   public wxDropTarget
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION METHODS
    //////////////////////////////////////////////////////////////////////////

    Drop();
    void init();
    ~Drop();

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
    void draw(wxDC& dc);


    void updateDropArea(wxPoint p);

private:
    wxRect mDropArea;

};

}} // namespace

#endif // DROP_H