// Copyright 2013 Eric Raijmakers.
//
// This file is part of Vidiot.
//
// Vidiot is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Vidiot is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Vidiot. If not, see <http://www.gnu.org/licenses/>.

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
