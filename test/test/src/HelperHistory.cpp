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

#include "Test.h"

#pragma warning(disable:4996)

namespace test {

//////////////////////////////////////////////////////////////////////////
// HistoryCommandAsserter
//////////////////////////////////////////////////////////////////////////

HistoryCommandAsserter::~HistoryCommandAsserter()
{
    // Ensures that any 'extra' commands do not go unnoticed.
    ASSERT(mCurrentCommandSeen);
}

void HistoryCommandAsserter::CommandMismatch(size_t mSkip, size_t mIndex, const std::type_info& expectedtype)
{
    size_t Skip{ mSkip };
    size_t Index{ mIndex };
    auto convert = [](const std::type_info& info) -> std::string
    {
        std::string theName{ info.name() };
        return theName.substr(theName.find_last_of(':') + 1);
    };

    std::string Expected{ convert(expectedtype) };
    LogVar("History type error", __FILE__, __LINE__,__FUNCTION__).LOGVAR_A(Skip)(Index)(Expected);
}

} // namespace
