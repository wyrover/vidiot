#include "ProjectEventAddAsset.h"
#include "UtilLog.h"

wxDEFINE_EVENT(PROJECT_EVENT_ADD_ASSET, ProjectEventAddAsset);

ProjectEventAddAsset::ProjectEventAddAsset(wxEventType eventType, model::ProjectViewPtr parent, model::ProjectViewPtr node)
:   ProjectEvent(eventType, node)
,   mParent(parent)
{
    VAR_INFO(parent)(node);
}

ProjectEventAddAsset::ProjectEventAddAsset(const ProjectEventAddAsset& other)
:   ProjectEvent(other)
,   mParent(other.mParent)
{
}
const model::ProjectViewPtr ProjectEventAddAsset::getParent() 
{ 
    return mParent; 
}

wxEvent* ProjectEventAddAsset::Clone() const 
{ 
    return new ProjectEventAddAsset(*this); 
}