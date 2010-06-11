#include "ProjectEventOpenProject.h"
#include "Folder.h"

wxDEFINE_EVENT(PROJECT_EVENT_OPEN_PROJECT, ProjectEventOpenProject);

ProjectEventOpenProject::ProjectEventOpenProject(wxEventType eventType, model::Project* project)
:   ProjectEvent(eventType, project->getRoot())
,   mProject(project)
{
    VAR_INFO(project);
}

ProjectEventOpenProject::ProjectEventOpenProject(const ProjectEventOpenProject& other)
:   ProjectEvent(other)
,   mProject(other.mProject)
{
}
model::Project* ProjectEventOpenProject::getProject() const
{ 
    return mProject; 
}

wxEvent* ProjectEventOpenProject::Clone() const 
{ 
    return new ProjectEventOpenProject(*this); 
}
