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

#ifndef FIXTURE_CONFIG_H
#define FIXTURE_CONFIG_H

namespace test {

/// This fixture may only be used when a GUI is also used.
class FixtureConfig
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    FixtureConfig();
    virtual ~FixtureConfig();

    //////////////////////////////////////////////////////////////////////////
    // SET
    //////////////////////////////////////////////////////////////////////////

    FixtureConfig& SetDefaults();
    FixtureConfig& SnapToClips(bool snap);
    FixtureConfig& SnapToCursor(bool snap);
};

} // namespace

#endif // FIXTURE_CONFIG_H