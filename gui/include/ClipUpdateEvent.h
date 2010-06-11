#ifndef GUI_CLIP_UPDATE_EVENT_H
#define GUI_CLIP_UPDATE_EVENT_H

#include <wx/event.h>
#include <wx/gdicmn.h>
#include "GuiPtr.h"

//////////////////////////////////////////////////////////////////////////

class ClipUpdateEvent 
    :   public wxEvent
{
public:
    ClipUpdateEvent(GuiTimeLineClipPtr clip, bool selectionChanged = false);
    ClipUpdateEvent(const ClipUpdateEvent& other);
    virtual wxEvent* Clone() const;
    GuiTimeLineClipPtr clip();

    bool selectionChanged() const;
private:
    GuiTimeLineClipPtr mClip;
    bool mSelectionChanged;

};

wxDECLARE_EVENT(CLIP_UPDATE_EVENT, ClipUpdateEvent);

//////////////////////////////////////////////////////////////////////////

class ClipSelectionEvent 
    :   public wxEvent
{
public:
    ClipSelectionEvent(GuiTimeLineClipPtr clip, bool selected);
    ClipSelectionEvent(const ClipSelectionEvent& other);
    virtual wxEvent* Clone() const;
    GuiTimeLineClipPtr clip();
    bool isSelected() const;
private:
    GuiTimeLineClipPtr mClip;
    bool mSelected;
};

wxDECLARE_EVENT(CLIP_SELECTION_EVENT, ClipSelectionEvent);

#endif // GUI_CLIP_UPDATE_EVENT_H