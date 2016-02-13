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

#include "Preview.h"

#include "Player.h"
#include "Timeline.h"

namespace gui {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

Preview::Preview(wxWindow* parent)
:   wxPanel(parent)
,   mPlayer()
,   mPlayers()
{
    LOG_INFO;

    wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(sizer);
}

Preview::~Preview()
{
}

//////////////////////////////////////////////////////////////////////////
// TO/FROM OTHER WIDGETS
//////////////////////////////////////////////////////////////////////////

Player* Preview::openTimeline(const model::SequencePtr& sequence, timeline::Timeline* timeline)
{
    ASSERT_MAP_CONTAINS_NOT(mPlayers,timeline);
    Player* newplayer = new Player(this, sequence, timeline);
    mPlayers[timeline] = newplayer;
    GetSizer()->Add(newplayer,wxSizerFlags(1).Expand());
    selectTimeline(timeline);
    return newplayer;
}

void Preview::closeTimeline(timeline::Timeline* timeline)
{
    ASSERT(mPlayer);
    ASSERT_MAP_CONTAINS(mPlayers,timeline);
    selectTimeline(0);
    Player* player = mPlayers[timeline];
    hide(player);
    mPlayers.erase(timeline);
    GetSizer()->Detach(player);
    if (mPlayers.size() > 0)
    {
        selectTimeline(mPlayers.begin()->first);
    }
    else
    {
        mPlayer = 0;
    }
    delete player;
}

void Preview::selectTimeline(timeline::Timeline* timeline)
{
	if (timeline == nullptr)
	{
	    hide(mPlayer);
	}
	else
	{
        ASSERT_MAP_CONTAINS(mPlayers,timeline);
		if (mPlayer != mPlayers[timeline])
		{
			hide(mPlayer);
			mPlayer = mPlayers[timeline];
			GetSizer()->Show(mPlayer);
		}
	}
    GetSizer()->Layout();
}

//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////

void Preview::play()
{
    if (mPlayer)
    {
        mPlayer->play();
    }
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void Preview::hide(Player* player)
{
    if (player)
    {
        player->pause();
        GetSizer()->Hide(player);
    }
}

} // namespace