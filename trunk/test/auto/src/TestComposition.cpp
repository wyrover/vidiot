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

#include "DetailsClip.h"

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
    triggerMenu(ID_ADDVIDEOTRACK);
    DragToTrack(1,VideoClip(0,3),model::IClipPtr());
    Drag(From(Center(VideoClip(1,1))).To(wxPoint(HCenter(VideoClip(0,4)),VCenter(VideoClip(1,1)))));
    ClickTopLeft(DetailsClipView()->getScalingSlider()); // Give focus
    TypeN(3,WXK_PAGEUP);
    waitForIdle();
    ClickTopLeft(DetailsClipView()->getPositionXSlider()); // Give focus
    TypeN(4,WXK_PAGEDOWN);
    ClickTopLeft(DetailsClipView()->getPositionYSlider()); // Give focus
    TypeN(15,WXK_PAGEDOWN);
    ClickTopLeft(DetailsClipView()->getOpacitySlider()); // Give focus
    TypeN(6,WXK_PAGEUP);
    Click(Center(VideoClip(0,4)));
    ClickTopLeft(DetailsClipView()->getScalingSlider()); // Give focus
    TypeN(5,WXK_PAGEUP);
    ClickTopLeft(DetailsClipView()->getPositionYSlider()); // Give focus
    TypeN(8,WXK_PAGEUP);
    ClickTopLeft(DetailsClipView()->getOpacitySlider()); // Give focus
    TypeN(3,WXK_PAGEUP);
    Scrub(HCenter(VideoClip(0,4))-5,HCenter(VideoClip(0,4))+5);
    Play(HCenter(VideoClip(0,4)),100);
    Undo(6);
}

void TestComposition::testAudioComposition()
{
    StartTestSuite();
    triggerMenu(ID_ADDAUDIOTRACK);
    DragToTrack(1,model::IClipPtr(),AudioClip(0,3));
    Drag(From(Center(AudioClip(1,1))).To(wxPoint(HCenter(AudioClip(0,4)),VCenter(AudioClip(1,1)))));
    Play(HCenter(AudioClip(1,1)),1500);
}

} // namespace