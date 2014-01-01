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

#ifndef HELPER_PLAYBACK_H
#define HELPER_PLAYBACK_H

namespace gui {
class PlaybackActiveEvent;
}

namespace test {

class WaitForPlayback 
    :   public wxEvtHandler // MUST BE FIRST INHERITED CLASS FOR WXWIDGETS EVENTS TO BE RECEIVED.
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    WaitForPlayback(bool waitForStart);
    ~WaitForPlayback();

    //////////////////////////////////////////////////////////////////////////
    // WAIT
    //////////////////////////////////////////////////////////////////////////

    void wait();

private:

    //////////////////////////////////////////////////////////////////////////
    // EVENT FROM PLAYER
    //////////////////////////////////////////////////////////////////////////

    void onPlaybackActive(gui::PlaybackActiveEvent& event);

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    bool mWaitForStart;
    bool mDone;
    boost::condition_variable mCondition;
    boost::mutex mMutex;
};

struct WaitForPlaybackStarted : public WaitForPlayback
{
    WaitForPlaybackStarted()
        :   WaitForPlayback(true)
    {
    }
};

struct WaitForPlaybackStopped : public WaitForPlayback
{
    WaitForPlaybackStopped()
        :   WaitForPlayback(false)
    {
    }
};

} // namespace

#endif // HELPER_PLAYBACK_H