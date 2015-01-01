// Copyright 2013-2015 Eric Raijmakers.
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

#include "TestConvert.h"

namespace test {

//////////////////////////////////////////////////////////////////////////
// TEST CASES
//////////////////////////////////////////////////////////////////////////

void TestConvert::testTimeConversions()
{
    StartTestSuite();

    {
        model::Properties properties(FrameRate::s25p);
        ASSERT_EQUALS(model::Convert::ptsToHumanReadibleString(0),"00.000");
        ASSERT_EQUALS(model::Convert::ptsToHumanReadibleString(25),"01.000");
        ASSERT_EQUALS(model::Convert::ptsToHumanReadibleString(50),"02.000");
        ASSERT_EQUALS(model::Convert::ptsToHumanReadibleString(500),"20.000");
        ASSERT_EQUALS(model::Convert::ptsToHumanReadibleString(5000),"03:20.000");
        ASSERT_EQUALS(model::Convert::ptsToHumanReadibleString(500000),"05:33:20.000");
        ASSERT_EQUALS(model::Convert::ptsToHumanReadibleString(3),"00.120");
        ASSERT_EQUALS(model::Convert::ptsToHumanReadibleString(27),"01.080");
        ASSERT_EQUALS(model::Convert::ptsToHumanReadibleString(60),"02.400");
        ASSERT_EQUALS(model::Convert::ptsToHumanReadibleString(522),"20.880");
        ASSERT_EQUALS(model::Convert::ptsToHumanReadibleString(5007),"03:20.280");
        ASSERT_EQUALS(model::Convert::ptsToHumanReadibleString(500024),"05:33:20.960");
    }
}

void TestConvert::testIntegerConversions()
{
     StartTestSuite();

     ASSERT_ZERO(removeRemainder(36500,2) % 2);
     ASSERT_ZERO(removeRemainder(36501,2) % 2);
     ASSERT_ZERO(removeRemainder(36502,2) % 2);
     ASSERT_ZERO(removeRemainder(36503,2) % 2);
}

} // namespace