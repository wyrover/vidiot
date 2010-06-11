#include "GuiEvent.h"

GuiEvent::GuiEvent(wxEventType eventType)
:   wxEvent(wxID_ANY, eventType)
{
}
GuiEvent::GuiEvent(const GuiEvent& other)
:   wxEvent(other)
{
}

GuiEvent::~GuiEvent()
{
}
