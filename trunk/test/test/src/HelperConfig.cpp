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

#include "HelperConfig.h"

#include "Config.h"
#include "HelperApplication.h"

namespace test {

//////////////////////////////////////////////////////////////////////////
// TEMPORARY OVERRULES
//////////////////////////////////////////////////////////////////////////

ConfigOverruleBool::ConfigOverruleBool(wxString path, bool temporaryvalue)
    :   mPath(path)
    ,   mOriginalValue(Config::ReadBool(path))
    ,   mTemporaryValue(temporaryvalue)
{
    Config::WriteBool(mPath,mTemporaryValue);
    waitForIdle();
}

ConfigOverruleBool::~ConfigOverruleBool()
{
    Config::WriteBool(mPath,mOriginalValue);
    waitForIdle();
}

ConfigOverruleLong::ConfigOverruleLong(wxString path, long temporaryvalue)
    :   mPath(path)
    ,   mOriginalValue(Config::ReadLong(path))
    ,   mTemporaryValue(temporaryvalue)
{
    Config::WriteLong(mPath,mTemporaryValue);
    waitForIdle();
}

ConfigOverruleLong::~ConfigOverruleLong()
{
    Config::WriteLong(mPath,mOriginalValue);
    waitForIdle();
}

ConfigOverruleString::ConfigOverruleString(wxString path, wxString temporaryvalue)
    :   mPath(path)
    ,   mOriginalValue(Config::ReadString(path))
    ,   mTemporaryValue(temporaryvalue)
{
    Config::WriteString(mPath,mTemporaryValue);
    waitForIdle();
}

ConfigOverruleString::~ConfigOverruleString()
{
    Config::WriteString(mPath,mOriginalValue);
    waitForIdle();
}

} // namespace