// Copyright 2013,2014 Eric Raijmakers.
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

#ifndef MODEL_I_CONTROL_H
#define MODEL_I_CONTROL_H

#include "UtilInt.h"

namespace model {

class IControl
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    virtual ~IControl() {};

    //////////////////////////////////////////////////////////////////////////
    // ICONTROL
    //////////////////////////////////////////////////////////////////////////

    virtual pts getLength() const = 0;
    virtual void moveTo(pts position) = 0;
    virtual wxString getDescription() const = 0; ///< Not called getName() to avoid conflict with Node::getName() for the Sequence class.
    virtual void clean() = 0;                    ///< Will be called to clean up resources. Used for minimizing required resources for the undo history.
};

} // namespace

#endif // MODEL_I_CONTROL_H