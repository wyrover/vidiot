// Copyright 2013-2016 Eric Raijmakers.
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

#pragma once

#include "Config.h"

namespace test {

/// Creating an overrule object temporarily overrules the value of a Config setting.
/// When the object is created, the temporary value is set.
/// When the object is destroyed, the original value is reset.
/// \note Changing config settings is done by accessing the application's Config object directly, not via UI actions.
template <typename TYPE>
class ConfigOverrule
{
public:
    ConfigOverrule(wxString path, TYPE temporaryvalue)
        : mPath(path)
        , mOriginalValue(Config::get().read<TYPE>(path))
        , mTemporaryValue(temporaryvalue)
    {
        util::thread::RunInMainAndWait([this] { Config::get().write(mPath, mTemporaryValue); });
    }

    ~ConfigOverrule()
    {
        util::thread::RunInMainAndWait([this] { Config::get().write(mPath, mOriginalValue); });
    }
private:
    wxString mPath;
    TYPE mOriginalValue;
    TYPE mTemporaryValue;
};

template <typename ENUM>
void CheckConfigEnum(wxString key, ENUM expected)
{
    ENUM current = util::thread::RunInMainReturning<ENUM>([key]
    {
        return Config::get().ReadEnum<ENUM>(key);
    });
    ASSERT_EQUALS(current, expected)(key);
}
#define ASSERT_CONFIG_ENUM(type, key, expected) CheckConfigEnum<type>(key,expected)

} // namespace
