#ifndef PROJECT_EVENT_CLOSE_PROJECT_H
#define PROJECT_EVENT_CLOSE_PROJECT_H

#include "Project.h"
#include "ProjectEvent.h"

class ProjectEventCloseProject : public ProjectEvent
{
public:
    ProjectEventCloseProject(wxEventType eventType, model::Project* project);
    ProjectEventCloseProject(const ProjectEventCloseProject& other);
    virtual wxEvent* Clone() const;

    model::Project* getProject() const;

private:
    model::Project* mProject;
};

wxDECLARE_EVENT(PROJECT_EVENT_CLOSE_PROJECT, ProjectEventCloseProject);

#endif PROJECT_EVENT_CLOSE_PROJECT_H
