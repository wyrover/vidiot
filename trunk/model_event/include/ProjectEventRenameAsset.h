#ifndef PROJECT_EVENT_RENAME_ASSET_H
#define PROJECT_EVENT_RENAME_ASSET_H

#include "ProjectEvent.h"

class ProjectEventRenameAsset : public ProjectEvent
{
public:
    ProjectEventRenameAsset(wxEventType eventType, model::ProjectViewPtr node, wxString newName);
    ProjectEventRenameAsset(const ProjectEventRenameAsset& other);
    const wxString getNewName();
    virtual wxEvent* Clone() const;
private:
    const wxString mNewName;
};

wxDECLARE_EVENT(PROJECT_EVENT_RENAME_ASSET, ProjectEventRenameAsset);

#endif PROJECT_EVENT_RENAME_ASSET_H
