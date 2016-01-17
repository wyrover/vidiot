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

#include "TestConvert.h"

#include "VideoTransitionHelpers.h"

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

void TestConvert::testFactorSliderConversions()
{
    StartTestSuite();

    ASSERT_EQUALS(gui::timeline::DetailsClip::sliderValueToFactor(19900), rational64(100,1));
    ASSERT_EQUALS(gui::timeline::DetailsClip::sliderValueToFactor(14900), rational64(50,1));
    ASSERT_EQUALS(gui::timeline::DetailsClip::sliderValueToFactor(11000), rational64(11,1));
    ASSERT_EQUALS(gui::timeline::DetailsClip::sliderValueToFactor(10100), rational64(2,1));
    ASSERT_EQUALS(gui::timeline::DetailsClip::sliderValueToFactor(10001), rational64(101,100));
    ASSERT_EQUALS(gui::timeline::DetailsClip::sliderValueToFactor(10000), rational64(1,1));
    ASSERT_EQUALS(gui::timeline::DetailsClip::sliderValueToFactor(5000), rational64(1,2));
    ASSERT_EQUALS(gui::timeline::DetailsClip::sliderValueToFactor(2000), rational64(1,5));
    ASSERT_EQUALS(gui::timeline::DetailsClip::sliderValueToFactor(100), rational64(1,100));
    
    ASSERT_EQUALS(gui::timeline::DetailsClip::factorToSliderValue(rational64(100,1)), 19900);
    ASSERT_EQUALS(gui::timeline::DetailsClip::factorToSliderValue(rational64(50,1)), 14900);
    ASSERT_EQUALS(gui::timeline::DetailsClip::factorToSliderValue(rational64(11,1)), 11000);
    ASSERT_EQUALS(gui::timeline::DetailsClip::factorToSliderValue(rational64(101,100)), 10001);
    ASSERT_EQUALS(gui::timeline::DetailsClip::factorToSliderValue(rational64(2,1)), 10100);
    ASSERT_EQUALS(gui::timeline::DetailsClip::factorToSliderValue(rational64(1,1)), 10000);
    ASSERT_EQUALS(gui::timeline::DetailsClip::factorToSliderValue(rational64(1,2)), 5000);
    ASSERT_EQUALS(gui::timeline::DetailsClip::factorToSliderValue(rational64(1,5)), 2000);
    ASSERT_EQUALS(gui::timeline::DetailsClip::factorToSliderValue(rational64(1,100)), 100);
}

void TestConvert::testAngleComputation()
{
    StartTestSuite();

    ASSERT_EQUALS(model::video::transition::angularDistance(100, 100, 100, 0), 0);
    ASSERT_EQUALS(model::video::transition::angularDistance(100, 100, 100, 50), 0);

    ASSERT_EQUALS(model::video::transition::angularDistance(100, 100, 200, 0), 45);
    ASSERT_EQUALS(model::video::transition::angularDistance(100, 100, 150, 50), 45);

    ASSERT_EQUALS(model::video::transition::angularDistance(100, 100, 200, 100), 90);
    ASSERT_EQUALS(model::video::transition::angularDistance(100, 100, 150, 100), 90);

    ASSERT_EQUALS(model::video::transition::angularDistance(100, 100, 200, 200), 135);
    ASSERT_EQUALS(model::video::transition::angularDistance(100, 100, 150, 150), 135);

    ASSERT_EQUALS(model::video::transition::angularDistance(100, 100, 100, 200), 180);
    ASSERT_EQUALS(model::video::transition::angularDistance(100, 100, 100, 150), 180);

    ASSERT_EQUALS(model::video::transition::angularDistance(100, 100, 0, 200), 225);
    ASSERT_EQUALS(model::video::transition::angularDistance(100, 100, 50, 150), 225);

    ASSERT_EQUALS(model::video::transition::angularDistance(100, 100, 0, 100), 270);
    ASSERT_EQUALS(model::video::transition::angularDistance(100, 100, 50, 100), 270);

    ASSERT_EQUALS(model::video::transition::angularDistance(100, 100, 0, 0), 315);
    ASSERT_EQUALS(model::video::transition::angularDistance(100, 100, 50, 50), 315);

    for (int x = 0; x < 200; ++x)
    {
        for (int y = 0; y < 200; ++y)
        {
            int distance{ model::video::transition::angularDistance(100,100,x,y) };
            ASSERT_LESS_THAN(distance, 360);
            ASSERT_MORE_THAN_EQUALS(distance, -1);
        }
    }
}

} // namespace