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

#ifndef TEST_ONCE_H
#define TEST_ONCE_H

namespace test
{

class TestRender : public CxxTest::TestSuite // Must be on same line as class definition. Otherwise 'No tests defined error
    ,   public SuiteCreator<TestRender>
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    virtual void setUp();       ///< Called before each test.
    virtual void tearDown();    ///< Called after each test.

    //////////////////////////////////////////////////////////////////////////
    // TEST CASES
    //////////////////////////////////////////////////////////////////////////

    void testChangeRenderSettings();
    void testRenderingSplit();

    /// For each video codec, render part of the timeline, and then playback
    /// the generated file.
    void testRenderingCodecs();

    /// This was a bug once. Caused by the clone not having the proper clip
    /// administration (getPrev/getNext failed when the transition started
    /// generating frames/chunks).
    void testRenderingTransition();

    /// This was a bug once. Caused by the empty clip not returning an image.
    /// The rendering code dereferenced the returned image ptr without
    /// checking for 0.
    void testRenderingEmptyClip();

    void testRenderingTransformedClip();

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    FixtureProject mProjectFixture;

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    void RenderTimelineInto(const wxFileName& path, int lengthInS = 1);
    void PlaybackRenderedTimeline(const wxFileName& path, pixel start = 2, milliseconds t = 600);
    void RenderAndPlaybackCurrentTimeline(int renderedlengthInS = 1, pixel playbackStart = 2, milliseconds playbackLength = 600);

};

}
using namespace test;

#endif