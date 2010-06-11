#ifndef PROJECT_EVENT_OPEN_PROJECT_H
#define PROJECT_EVENT_OPEN_PROJECT_H

#include "Project.h"
#include "ProjectEvent.h"

class ProjectEventOpenProject : public ProjectEvent
{
public:
    ProjectEventOpenProject(wxEventType eventType, model::Project* project);
    ProjectEventOpenProject(const ProjectEventOpenProject& other);
    virtual wxEvent* Clone() const;

    model::Project* getProject() const;

private:
    model::Project* mProject;
};

wxDECLARE_EVENT(PROJECT_EVENT_OPEN_PROJECT, ProjectEventOpenProject);

#endif PROJECT_EVENT_OPEN_PROJECT_H
