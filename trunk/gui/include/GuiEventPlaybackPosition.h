#ifndef GUI_EVENT_PLAYBACK_POSITION_H
#define GUI_EVENT_PLAYBACK_POSITION_H

#include <boost/cstdint.hpp>
#include "GuiEvent.h"

class GuiEventPlaybackPosition : public GuiEvent
{
public:
    GuiEventPlaybackPosition(boost::int64_t pts);
    GuiEventPlaybackPosition(const GuiEventPlaybackPosition& other);
    virtual wxEvent* Clone() const;
    boost::int64_t getPts() const;
private:
    boost::int64_t mPts;
};

wxDECLARE_EVENT(GUI_EVENT_PLAYBACK_POSITION, GuiEventPlaybackPosition);

#endif GUI_EVENT_PLAYBACK_POSITION_H
