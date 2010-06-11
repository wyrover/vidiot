#include "GuiEventPlaybackPosition.h"

wxDEFINE_EVENT(GUI_EVENT_PLAYBACK_POSITION, GuiEventPlaybackPosition);

GuiEventPlaybackPosition::GuiEventPlaybackPosition(boost::int64_t pts)
:   GuiEvent(GUI_EVENT_PLAYBACK_POSITION)
,   mPts(pts)
{
}

GuiEventPlaybackPosition::GuiEventPlaybackPosition(const GuiEventPlaybackPosition& other)
:   GuiEvent(other)
,   mPts(other.mPts)
{
}
wxEvent* GuiEventPlaybackPosition::Clone() const 
{ 
    return new GuiEventPlaybackPosition(*this); 
}

boost::int64_t GuiEventPlaybackPosition::getPts() const
{
    return mPts;
}
