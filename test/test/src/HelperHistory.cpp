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

#include "Test.h"

#pragma warning(disable:4996)

namespace test {

//////////////////////////////////////////////////////////////////////////
// HistoryCommandAsserter
//////////////////////////////////////////////////////////////////////////


HistoryCommandAsserter::~HistoryCommandAsserter()
{
    std::vector<wxCommand*> commands{ model::CommandProcessor::get().getUndoHistory() };

    wxStrings Actual;
    for (wxCommand* c : commands)
    {
        Actual.emplace_back(getName(typeid(*c)));
    }

    ASSERT_LESS_THAN_EQUALS(Expected.size(), Actual.size())(Expected)(Actual)(commands);
    size_t offset{ Actual.size() - Expected.size() };

    Actual.erase(Actual.begin(), Actual.begin() + offset);
    ASSERT_EQUALS(Actual, Expected);
}

wxString HistoryCommandAsserter::getName(const std::type_info& info)
{
    wxString theName{ info.name() };
    return theName.substr(theName.find_last_of(':') + 1);
};

} // namespace
