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

#include "FixtureConfig.h"

#include "HelperApplication.h"
#include "HelperWindow.h"
#include "HelperTimeline.h"
#include "ids.h"

namespace test {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

FixtureConfig::FixtureConfig()
{
}

FixtureConfig::~FixtureConfig()
{
}

//////////////////////////////////////////////////////////////////////////
// SET
//////////////////////////////////////////////////////////////////////////

FixtureConfig& FixtureConfig::SetDefaults()
{
    return SnapToClips(false).SnapToCursor(false);
}

FixtureConfig& FixtureConfig::SnapToClips(bool snap)
{

    checkMenu(ID_SNAP_CLIPS, snap);
    waitForIdle();
    return *this;
}
FixtureConfig& FixtureConfig::SnapToCursor(bool snap)
{
    checkMenu(ID_SNAP_CURSOR, snap);
    waitForIdle();
    return *this;
}

} // namespace