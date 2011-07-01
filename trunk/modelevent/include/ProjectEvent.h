#ifndef PROJECT_EVENT_H
#define PROJECT_EVENT_H

#include "UtilEvent.h"

namespace model {

class Project;

DECLARE_EVENT(EVENT_OPEN_PROJECT,   EventOpenProject,   model::Project*);
DECLARE_EVENT(EVENT_CLOSE_PROJECT,  EventCloseProject,  model::Project*);
DECLARE_EVENT(EVENT_RENAME_PROJECT, EventRenameProject,  model::Project*);

} // namespace

#endif // PROJECT_EVENT_H
