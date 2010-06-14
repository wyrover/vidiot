#include "ClipUpdateEvent.h"

//////////////////////////////////////////////////////////////////////////

wxDEFINE_EVENT(CLIP_UPDATE_EVENT, ClipUpdateEvent);

ClipUpdateEvent::ClipUpdateEvent(GuiTimeLineClipPtr clip, bool selectionChanged)
:   wxEvent(wxID_ANY,CLIP_UPDATE_EVENT)
,   mClip(clip)
,   mSelectionChanged(false)
{
}

ClipUpdateEvent::ClipUpdateEvent(const ClipUpdateEvent& other)
:   wxEvent(other)
,   mClip(other.mClip)
,   mSelectionChanged(other.mSelectionChanged)
{
}

wxEvent* ClipUpdateEvent::Clone() const
{
    return new ClipUpdateEvent(*this);
}

GuiTimeLineClipPtr ClipUpdateEvent::clip()
{
    return mClip;
}

bool ClipUpdateEvent::selectionChanged() const
{
    return mSelectionChanged;
}
