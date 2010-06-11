#ifndef GUI_EVENT_H
#define GUI_EVENT_H

#include "wx/event.h"

class GuiEvent : public wxEvent
{
public:
    GuiEvent(wxEventType eventType);
    GuiEvent(const GuiEvent& other);
    ~GuiEvent();
};

#endif GUI_EVENT_H
