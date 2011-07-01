#include "ProjectEvent.h"

namespace model {

DEFINE_EVENT(EVENT_OPEN_PROJECT,    EventOpenProject,   model::Project*);
DEFINE_EVENT(EVENT_CLOSE_PROJECT,   EventCloseProject,  model::Project*);
DEFINE_EVENT(EVENT_RENAME_PROJECT,  EventRenameProject, model::Project*);

} //namespace
