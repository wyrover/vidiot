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

#ifndef FIXTURE_GUI_H
#define FIXTURE_GUI_H

#include "Application.h"

namespace test {

class HelperTestSuite;

/// Fixture for the complete GUI. The complete application is started, with the
/// main thread running in a separate thread so that tests can be ran from the main
/// application thread. This fixture also ensures that test execution waits
/// until that event loop is running properly (the wxWidgets part). The implementation
/// also ensures that a new window is used for each new test.
class FixtureGui
    :   public CxxTest::GlobalFixture
    ,   public IEventLoopListener
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    static FixtureGui sInstance;

    FixtureGui();
    virtual ~FixtureGui();

    virtual bool setUpWorld();      ///< Called before first test. Starts Application() and the main wxWidgets thread
    virtual bool tearDownWorld();   ///< Called after last test. Closes main wxWidgets thread and Application()

    virtual bool setUp();       ///< Called before each test. Starts the window.
    virtual bool tearDown();    ///< Called after each test. Closes the window.

    /// No behaviour but the inclusion of this method in any test suite ensures
    /// that the test fixture (which is a global object) is not discarded during
    /// compilation (linking?) due to not being referenced anywhere.
    static void start();

    /// If true, actual mouse move/keyboard presses are used in the tests.
    /// If false, the events resulting from the mouse move/keyboard presses
    /// are 'generated'.
    static bool UseRealUiEvents;

    //////////////////////////////////////////////////////////////////////////
    // IEventLoopListener
    //////////////////////////////////////////////////////////////////////////

    void onEventLoopEnter();    ///< Receive notification that the OnRun thread is running the wxWidgets event loop for the (re)started Window


private:

    //////////////////////////////////////////////////////////////////////////
    // MAIN WXWIDGETS THREAD
    //////////////////////////////////////////////////////////////////////////

    void mainThread();

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    boost::scoped_ptr<boost::thread> mThread;

    boost::barrier mBarrierConfigRead;
    boost::barrier mBarrierStart;
    boost::barrier mBarrierStarted;
    boost::barrier mBarrierStopped;

    boost::mutex mEndMutex;

    bool mEnd;
    bool mStartingMainThread;

    long mStartTime;

    boost::shared_ptr<HelperTestSuite> mHelperTestSuite;
};

} // namespace

#endif
