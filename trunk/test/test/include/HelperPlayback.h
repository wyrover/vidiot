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

#pragma once

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

/// 1. Start playback from the given position
/// 2. Wait until ms has expired, stop playback
/// 3. wait until playback has stopped.
void Play(pixel from, int ms);

/// 1. Start playback from the current position
/// 2. Wait until ms has expired, stop playback.
/// 3. wait until playback has stopped.
void Play(int ms);

} // namespace
