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

#include "TestPlayback.h"

#include "HelperApplication.h"
#include "HelperPlayback.h"

namespace test {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

void TestPlayback::setUp()
{
    mProjectFixture.init();
}

void TestPlayback::tearDown()
{
    mProjectFixture.destroy();
}

//////////////////////////////////////////////////////////////////////////
// TEST CASES
//////////////////////////////////////////////////////////////////////////

void TestPlayback::testPlaybackUntilEndOfSequence()
{
    StartTestSuite();
    PositionCursor(RightPixel(VideoTrack(0)) - 5);
    WaitForPlayback playbackstarted(true);
    WaitForPlayback playbackstopped(false);
    Type(' ');
    playbackstarted.wait();
    playbackstopped.wait();
}

} // namespace