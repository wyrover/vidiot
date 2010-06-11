#ifndef PROJECT_EVENT_H
#define PROJECT_EVENT_H

#include "wx/event.h"
#include "AProjectViewNode.h"

class ProjectEvent : public wxEvent
{
public:
    ProjectEvent(wxEventType eventType, model::ProjectViewPtr node);
    ProjectEvent(const ProjectEvent& other);
    ~ProjectEvent();
    model::ProjectViewPtr getNode() const;
protected:
    const model::ProjectViewPtr mNode;
};

#endif PROJECT_EVENT_H
