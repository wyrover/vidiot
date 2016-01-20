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

#include "TestVideoTransitions.h"

namespace test {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

void TestVideoTransitions::setUp()
{
    mProjectFixture.init();
}

void TestVideoTransitions::tearDown()
{
    mProjectFixture.destroy();
}

//////////////////////////////////////////////////////////////////////////
// TEST CASES
//////////////////////////////////////////////////////////////////////////

void TestVideoTransitions::testVideoTransitions()
{
    auto setOpacity = [](int index, int opacity)
    {
        std::ostringstream os; os << "Set opacity: " << opacity;
        StartTest(os.str().c_str());
        TimelineSelectClips({});
        TimelineLeftClick(Center(VideoClip(0,index)));
        SetValue(DetailsClipView()->getOpacitySlider(), opacity); // Same as WXK_PAGEUP
        ASSERT_HISTORY_END(gui::timeline::cmd::EditClipDetails); // Verify that only one command object was added to the undo history
        ASSERT_EQUALS(DefaultVideoKeyFrame(VideoClip(0, index))->getOpacity(), opacity);
    };

    StartTestSuite();
    TimelineZoomIn(6);


    // Transition is always created after video clip 1.
    // The index below indicates the index of the resulting transition in the track.
    std::map<model::TransitionType, int> TransitionClipIndex
    {
        { model::TransitionTypeFadeIn, 1 },
        { model::TransitionTypeFadeOut, 2 },
        { model::TransitionTypeFadeInFromPrevious, 1 },
        { model::TransitionTypeFadeOutToNext, 2 },
    };

    for ( model::TransitionPtr t : model::video::VideoTransitionFactory::get().getAllPossibleTransitions() )
    {
        for (auto type_and_index : TransitionClipIndex)
        {
            if (t->supports(type_and_index.first))
            {
                model::TransitionType tt{ type_and_index.first };
                int ti{ type_and_index.second };
                StartTest(model::TransitionType_toString(tt) + " " + t->getDescription());
                util::thread::RunInMainAndWait([t, tt]() { gui::timeline::cmd::createTransition(getSequence(), VideoClip(0, 1), tt, t); });
                ASSERT(VideoClip(0, ti)->isA<model::Transition>());
                TimelineLeftClick(Center(VideoClip(0, ti))); // Open properties
                WaitForPlaybackStarted started;
                ButtonTriggerPressed(DetailsClipView()->getPlayButton());
                started.wait();
                pause(1000);
                WaitForPlaybackStopped stopped;
                ButtonTriggerPressed(DetailsClipView()->getPlayButton());
                stopped.wait();
                Scrub(-2 + LeftPixel(VideoClip(0, ti)), RightPixel(VideoClip(0, ti)) + 2);
                Play(-2 + LeftPixel(VideoClip(0, ti)), 250);
                StartTest(model::TransitionType_toString(tt) + " " + t->getDescription() + " (opacity)");
                setOpacity(ti - 1, 50);
                setOpacity(ti + 1, 200);
                Scrub(-2 + LeftPixel(VideoClip(0, ti)), RightPixel(VideoClip(0, ti)) + 2);
                // Important: Position cursor BEFORE the clip!
                //            Otherwise, for some transitions code is ran in the main thread and not in the buffer thread.
                //            Some transitions used calls that were not allowed in a secondary thread (config calls).
                Play(-2 + LeftPixel(VideoClip(0, ti)), 250);
                Undo(3);
            }
        }
    }
}

} // namespace
