#ifndef PROJECT_EVENT_ADD_ASSET_H
#define PROJECT_EVENT_ADD_ASSET_H

#include "ProjectEvent.h"

class ProjectEventAddAsset : public ProjectEvent
{
public:
    ProjectEventAddAsset(wxEventType eventType, model::ProjectViewPtr parent, model::ProjectViewPtr node);
    ProjectEventAddAsset(const ProjectEventAddAsset& other);
    const model::ProjectViewPtr getParent();
    virtual wxEvent* Clone() const;

private:
    const model::ProjectViewPtr mParent;
};

wxDECLARE_EVENT(PROJECT_EVENT_ADD_ASSET, ProjectEventAddAsset);

#endif PROJECT_EVENT_ADD_ASSET_H
