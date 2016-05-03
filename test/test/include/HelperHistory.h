// Copyright 2015-2016 Eric Raijmakers.
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

struct HistoryCommandAsserter
{
    HistoryCommandAsserter& HISTORYCOMMANDASSERTER_A;   ///< Helper, in order to be able to compile the code (HISTORYCOMMANDASSERTER_* macros)
    HistoryCommandAsserter& HISTORYCOMMANDASSERTER_B;   ///< Helper, in order to be able to compile the code (HISTORYCOMMANDASSERTER_* macros)

    /// Verify last command in history
    explicit HistoryCommandAsserter()
        : HISTORYCOMMANDASSERTER_A(*this)
        , HISTORYCOMMANDASSERTER_B(*this)
    {
    }

    HistoryCommandAsserter(const HistoryCommandAsserter&) = delete;
    HistoryCommandAsserter& operator=(const HistoryCommandAsserter&) = delete;

    ~HistoryCommandAsserter();

    template<class type>
    HistoryCommandAsserter& AssertEntry()
    {
        Expected.emplace_back(getName(typeid(type)));
        return *this;
    }

private:

    wxStrings Expected;

    wxString getName(const std::type_info& info);
};

#define HISTORYCOMMANDASSERTER_A(type) HISTORYCOMMANDASSERTER_OP(type, B)
#define HISTORYCOMMANDASSERTER_B(type) HISTORYCOMMANDASSERTER_OP(type, A)
#define HISTORYCOMMANDASSERTER_OP(type, next) HISTORYCOMMANDASSERTER_A.AssertEntry<type>().HISTORYCOMMANDASSERTER_ ## next
#define ASSERT_HISTORY_END HistoryCommandAsserter().HISTORYCOMMANDASSERTER_A

} // namespace
