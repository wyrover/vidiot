// Copyright 2015 Eric Raijmakers.
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

namespace test {

//////////////////////////////////////////////////////////////////////////
// HistoryCommandAsserter
//////////////////////////////////////////////////////////////////////////

struct HistoryCommandAsserter   // todo remove all boost::noncopyable  replace with =delete
{
    HistoryCommandAsserter& HISTORYCOMMANDASSERTER_A;   ///< Helper, in order to be able to compile the code (HISTORYCOMMANDASSERTER_* macros)
    HistoryCommandAsserter& HISTORYCOMMANDASSERTER_B;   ///< Helper, in order to be able to compile the code (HISTORYCOMMANDASSERTER_* macros)

    HistoryCommandAsserter(size_t skip)
        : HISTORYCOMMANDASSERTER_A(*this)
        , HISTORYCOMMANDASSERTER_B(*this)
        , mSkip(skip)
    {}

    ~HistoryCommandAsserter();

    template<class type>
    HistoryCommandAsserter& AssertEntry()
    {
        std::vector<wxCommand*> commands;
        for (wxCommand* c : model::CommandProcessor::get().GetCommands().AsVector<wxCommand*>())
        {
            commands.push_back(c);
        }
        size_t required{ mSkip + mIndex };
        ASSERT_LESS_THAN(required, commands.size());
        wxCommand* got{ commands.at(required) };
        ASSERT(!mCurrentCommandSeen);
        mCurrentCommandSeen = (got == getCurrentCommand());
        if (dynamic_cast<type*>(got) == nullptr)
        {
            CommandMismatch(mSkip, mIndex, typeid(type));
        }
        mIndex++;
        return *this;
    }

    void CommandMismatch(size_t mSkip, size_t mIndex, const std::type_info& expectedtype);

    size_t mSkip{ 0 };
    size_t mIndex{ 0 };
    bool mCurrentCommandSeen{ false };
};

#define HISTORYCOMMANDASSERTER_A(type) HISTORYCOMMANDASSERTER_OP(type, B)
#define HISTORYCOMMANDASSERTER_B(type) HISTORYCOMMANDASSERTER_OP(type, A)
#define HISTORYCOMMANDASSERTER_OP(type, next) HISTORYCOMMANDASSERTER_A.AssertEntry<type>().HISTORYCOMMANDASSERTER_ ## next
#define ASSERT_HISTORY_SKIP(skip) HistoryCommandAsserter(skip).HISTORYCOMMANDASSERTER_A
#define ASSERT_HISTORY ASSERT_HISTORY_SKIP(0)

} // namespace
