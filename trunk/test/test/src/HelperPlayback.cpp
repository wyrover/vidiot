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

#include "HelperPlayback.h"

#include "Player.h"
#include "VideoDisplayEvent.h"

namespace test {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

WaitForPlayback::WaitForPlayback(bool waitForStart)
    :   mWaitForStart(waitForStart)
    ,   mDone(false)
{
    getTimeline().getPlayer()->Bind(gui::EVENT_PLAYBACK_ACTIVE, &WaitForPlayback::onPlaybackActive, this);
}
WaitForPlayback::~WaitForPlayback()
{
    getTimeline().getPlayer()->Unbind(gui::EVENT_PLAYBACK_ACTIVE, &WaitForPlayback::onPlaybackActive, this);
}

//////////////////////////////////////////////////////////////////////////
// WAIT
//////////////////////////////////////////////////////////////////////////

void WaitForPlayback::wait()
{
    boost::mutex::scoped_lock lock(mMutex);
    while (!mDone)
    {
        mCondition.wait(lock);
    }
}
//////////////////////////////////////////////////////////////////////////
// EVENT FROM PLAYER
//////////////////////////////////////////////////////////////////////////

void WaitForPlayback::onPlaybackActive(gui::PlaybackActiveEvent& event)
{
    if (event.getValue() == mWaitForStart)
    {
        mDone = true;
        boost::mutex::scoped_lock lock(mMutex);
        mCondition.notify_all();
    }
    event.Skip();
}

} // namespace