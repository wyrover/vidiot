#include "ProjectEvent.h"
#include "UtilLog.h"

ProjectEvent::ProjectEvent(wxEventType eventType, model::ProjectViewPtr node)
:   wxEvent(wxID_ANY, eventType)
,   mNode(node)
{
}
ProjectEvent::ProjectEvent(const ProjectEvent& other)
:   wxEvent(other)
,   mNode(other.mNode)
{
}

ProjectEvent::~ProjectEvent()
{
}

model::ProjectViewPtr ProjectEvent::getNode() const
{ 
    return mNode; 
}
