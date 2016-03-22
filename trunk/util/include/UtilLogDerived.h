// Copyright 2016 Eric Raijmakers.
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

template <typename DERIVED, typename BASE>
std::ostream& logAs(std::ostream& os, const boost::shared_ptr<BASE>& obj)
{
    if (obj == nullptr)
    {
        os << "00000000";
    }
    else
    {
        os << *(boost::dynamic_pointer_cast<DERIVED>(obj));
    }
    return os;
}

template <typename DERIVED, typename BASE, typename KEY>
std::ostream& logAs(std::ostream& os, const std::map<KEY, boost::shared_ptr<BASE> >& obj)
{
    os << '{';
    for (auto kvp : obj)
    {
        os << '(' << kvp.first << ',';
        logAs<DERIVED>(os, kvp.second);
        os << ')';
    }
    os << '}';
    return os;
}
