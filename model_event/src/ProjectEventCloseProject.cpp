#include "ProjectEventCloseProject.h"
#include "Folder.h"

wxDEFINE_EVENT(PROJECT_EVENT_CLOSE_PROJECT, ProjectEventCloseProject);

ProjectEventCloseProject::ProjectEventCloseProject(wxEventType eventType, model::Project* project)
:   ProjectEvent(eventType, project->getRoot())
,   mProject(project)
{
    VAR_INFO(project);
}

ProjectEventCloseProject::ProjectEventCloseProject(const ProjectEventCloseProject& other)
:   ProjectEvent(other)
,   mProject(other.mProject)
{
}
model::Project* ProjectEventCloseProject::getProject() const
{ 
    return mProject; 
}

wxEvent* ProjectEventCloseProject::Clone() const 
{ 
    return new ProjectEventCloseProject(*this); 
}
