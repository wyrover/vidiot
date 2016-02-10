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

#include "CommandProcessor.h"

namespace model {

//////////////////////////////////////////////////////////////////////////
// wxCommandProcessor
//////////////////////////////////////////////////////////////////////////

bool CommandProcessor::Redo()
{
    LOG_INFO;
    ASSERT_MORE_THAN_ZERO(mRedo);
    mRedo--;
    auto players{ pausePlayers() };
    bool result{ wxCommandProcessor::Redo() };
    resumePlayers(players);
    return result;
}

bool CommandProcessor::Submit(wxCommand *command, bool storeIt)
{
    VAR_INFO(command)(storeIt);
    mRedo = 0;
    auto players{ pausePlayers() };
    bool result{ wxCommandProcessor::Submit(command,storeIt) };
    resumePlayers(players);
    return result;
}

bool CommandProcessor::Undo()
{
    LOG_INFO;
    mRedo++;
    auto players{ pausePlayers() };
    bool result{ wxCommandProcessor::Undo() };
    resumePlayers(players);
    return result;
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

std::vector<wxCommand*> CommandProcessor::getUndoHistory() const
{
    std::vector<wxCommand*> result;
    for (wxCommand* c : GetCommands().AsVector<wxCommand*>())
    {
        result.push_back(c);
    }

    // Remove any 'Undone' commands.
    ASSERT_LESS_THAN_EQUALS(mRedo, result.size())(result);
    size_t undone{ mRedo };
    while (undone-- > 0)
    {
        result.pop_back();
    }
    return result;
}


void CommandProcessor::registerPlayer(IPlayer* player)
{
    mPlayers.push_back(player);
}

void CommandProcessor::unregisterPlayer(IPlayer* player)
{
    auto it{ std::find(mPlayers.begin(), mPlayers.end(), player) };
    ASSERT(it != mPlayers.end())(mPlayers)(player);
    mPlayers.erase(it);
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

std::vector<std::pair<IPlayer*, ResumeInfo>> CommandProcessor::pausePlayers() const
{
    std::vector<std::pair<IPlayer*, ResumeInfo>> result;
    for (IPlayer* player : mPlayers)
    {
        result.push_back(std::make_pair(player, player->pause()));
    }
    return result;
}

void CommandProcessor::resumePlayers(std::vector<std::pair<IPlayer*, ResumeInfo>> players)
{
    for (auto kvp : players)
    {
        kvp.first->resume(kvp.second);
    }
}

} // namespace