#include "ProjectEventDeleteAsset.h"
#include "UtilLog.h"

wxDEFINE_EVENT(PROJECT_EVENT_DELETE_ASSET, ProjectEventDeleteAsset);

ProjectEventDeleteAsset::ProjectEventDeleteAsset(wxEventType eventType, model::ProjectViewPtr parent, model::ProjectViewPtr node)
:   ProjectEvent(eventType, node)
,   mParent(parent)
{
    VAR_INFO(parent)(node);
}

ProjectEventDeleteAsset::ProjectEventDeleteAsset(const ProjectEventDeleteAsset& other)
:   ProjectEvent(other)
,   mParent(other.mParent)
{
}

const model::ProjectViewPtr ProjectEventDeleteAsset::getParent() 
{ 
    return mParent; 
}

wxEvent* ProjectEventDeleteAsset::Clone() const 
{ 
    return new ProjectEventDeleteAsset(*this); 
}