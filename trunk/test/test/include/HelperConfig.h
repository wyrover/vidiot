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

#ifndef HELPER_CONFIG_H
#define HELPER_CONFIG_H

#include "Config.h"

namespace test {

/// Creating an overrule object temporarily overrules the (bool) value of a Config setting.
/// When the object is created, the temporary value is set.
/// When the object is destroyed, the original value is reset.
/// \note Changing config settings is done by accessing the application's Config object directly, not via UI actions.
class ConfigOverruleBool
{
public:
    ConfigOverruleBool(wxString path, bool temporaryvalue);
    virtual ~ConfigOverruleBool();
private:
    wxString mPath;
    bool mOriginalValue;
    bool mTemporaryValue;
};

/// Creating an overrule object temporarily overrules the (Long) value of a Config setting.
/// When the object is created, the temporary value is set.
/// When the object is destroyed, the original value is reset.
/// \note Changing config settings is done by accessing the application's Config object directly, not via UI actions.
class ConfigOverruleLong
{
public:
    ConfigOverruleLong(wxString path, long temporaryvalue);
    virtual ~ConfigOverruleLong();
private:
    wxString mPath;
    long mOriginalValue;
    long mTemporaryValue;
};

} // namespace

#endif // HELPER_CONFIG_H