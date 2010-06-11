#include "ProjectEventRenameAsset.h"
#include "UtilLog.h"

wxDEFINE_EVENT(PROJECT_EVENT_RENAME_ASSET, ProjectEventRenameAsset);

ProjectEventRenameAsset::ProjectEventRenameAsset(wxEventType eventType, model::ProjectViewPtr node, wxString newName)
:   ProjectEvent(eventType, node)
,   mNewName(newName)
{
    VAR_INFO(node)(newName);
}
ProjectEventRenameAsset::ProjectEventRenameAsset(const ProjectEventRenameAsset& other)
:   ProjectEvent(other)
,   mNewName(other.mNewName)
{
}
const wxString ProjectEventRenameAsset::getNewName() 
{ 
    return mNewName; 
}
wxEvent* ProjectEventRenameAsset::Clone() const 
{ 
    return new ProjectEventRenameAsset(*this); 
}