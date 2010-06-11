#ifndef PROJECT_EVENT_DELETE_ASSET_H
#define PROJECT_EVENT_DELETE_ASSET_H

#include "ProjectEvent.h"

class ProjectEventDeleteAsset : public ProjectEvent
{
public:
    ProjectEventDeleteAsset(wxEventType eventType, model::ProjectViewPtr parent, model::ProjectViewPtr node);
    ProjectEventDeleteAsset(const ProjectEventDeleteAsset& other);
    const model::ProjectViewPtr getParent();
    virtual wxEvent* Clone() const;
private:
    const model::ProjectViewPtr mParent;
};

wxDECLARE_EVENT(PROJECT_EVENT_DELETE_ASSET, ProjectEventDeleteAsset);

#endif PROJECT_EVENT_DELETE_ASSET_H
