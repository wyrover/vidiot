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

#include "TestComposition.h"

namespace test {

    //////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

void TestComposition::setUp()
{
    mProjectFixture.init();
}

void TestComposition::tearDown()
{
    mProjectFixture.destroy();
}

//////////////////////////////////////////////////////////////////////////
// TEST CASES
//////////////////////////////////////////////////////////////////////////

void TestComposition::testBlend()
{
    StartTestSuite();
    WindowTriggerMenu(ID_ADDVIDEOTRACK);
    TimelineDragToTrack(1,VideoClip(0,3),model::IClipPtr());
    TimelineDrag(From(Center(VideoClip(1,1))).To(wxPoint(HCenter(VideoClip(0,4)),VCenter(VideoClip(1,1)))));

    SetValue(DetailsClipView()->getScalingSlider(),7000); // Same as pressing 3 * PageUp
    SetValue(DetailsClipView()->getPositionXSlider(),232); // Same as pressing 4 * PageUp
    SetValue(DetailsClipView()->getPositionYSlider(),108); // Same as pressing 15 * PageDown
    SetValue(DetailsClipView()->getOpacitySlider(),195); // Same as pressing 6 * PageUp

    TimelineLeftClick(Center(VideoClip(0,4)));
    SetValue(DetailsClipView()->getScalingSlider(),5000); // Same as pressing 5 * PageUp
    SetValue(DetailsClipView()->getPositionYSlider(),180); // Same as pressing 8 * PageUp
    SetValue(DetailsClipView()->getOpacitySlider(),225); // Same as pressing 3 * PageUp

    Scrub(HCenter(VideoClip(0,4))-5,HCenter(VideoClip(0,4))+5);
    Play(HCenter(VideoClip(0,4)),100);

    Undo(6);
}

void TestComposition::testAudioComposition()
{
    StartTestSuite();
    WindowTriggerMenu(ID_ADDAUDIOTRACK);
    TimelineDragToTrack(1,model::IClipPtr(),AudioClip(0,3));
    TimelineDrag(From(Center(AudioClip(1,1))).To(wxPoint(HCenter(AudioClip(0,4)),VCenter(AudioClip(1,1)))));
    Play(HCenter(AudioClip(1,1)),1500);
}

} // namespace