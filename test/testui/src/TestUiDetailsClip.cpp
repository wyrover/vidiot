// Copyright 2014 Eric Raijmakers.
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

#include "TestUiDetailsClip.h"

namespace test {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

void TestUiDetailsClip::setUp()
{
    mProjectFixture.init();
}

void TestUiDetailsClip::tearDown()
{
    mProjectFixture.destroy();
}

//////////////////////////////////////////////////////////////////////////
// TEST CASES
//////////////////////////////////////////////////////////////////////////

void TestUiDetailsClip::testTransform_Boundaries()
{
    StartTestSuite();

    {
        StartTest("Scaling: Minimum scaling factor.");
        TimelineLeftClick(Center(VideoClip(0,5)));
        ASSERT_DETAILSCLIP(VideoClip(0,5));
        GiveKeyboardFocus(DetailsClipView()->getScalingSpin());
        KeyboardKeyPressN(7,WXK_DELETE); // Remove all characters
        KeyboardKeyPress('0'); // 0 will be replaced with 'min' value
        KeyboardKeyPress(WXK_TAB);
        ASSERT_CURRENT_COMMAND_TYPE<model::ChangeVideoClipTransform>(); // Verify that only one command object was added to the undo history
        ASSERT_CLIPPROPERTIES(VideoClip(0,5),model::VideoScalingCustom,boost::rational<int>(model::Constants::sScalingMin,model::Constants::sScalingPrecisionFactor),model::VideoAlignmentCenter,wxPoint(640,360),0); // The scaling spin buttons increment with 0.01, not 0.0001
        Undo();
    }
    {
        StartTest("Scaling: Maximum scaling factor.");
        TimelineLeftClick(Center(VideoClip(0,5)));
        ASSERT_DETAILSCLIP(VideoClip(0,5));
        GiveKeyboardFocus(DetailsClipView()->getScalingSpin());
        KeyboardKeyPressN(7,WXK_DELETE); // Remove all characters
        KeyboardKeyPressN(10,'9'); // 999999999 will be replaced with 'max' value
        KeyboardKeyPress(WXK_TAB);
        SetValue(DetailsClipView()->getScalingSpin(), 999999999); // 999999999 will be replaced with 'max' value
        ASSERT_CURRENT_COMMAND_TYPE<model::ChangeVideoClipTransform>(); // Verify that only one command object was added to the undo history
        ASSERT_CLIPPROPERTIES(VideoClip(0,5),model::VideoScalingCustom,boost::rational<int>(model::Constants::sScalingMax,model::Constants::sScalingPrecisionFactor),model::VideoAlignmentCenter,wxPoint(-5760,-3240),0); // The scaling spin buttons increment with 0.01, not 0.0001
        Undo();
    }
}

} // namespace
