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

#ifndef UTIL_LOG_BOOST_H
#define UTIL_LOG_BOOST_H

// Strange but true, some boost classes must be defined outside boost namespace
// (otherwise, got compile error) and some boost classes must be defined inside
// boost namespace (otherwise, got link error)...

template <class K, class L, class M>
std::ostream& operator<<(std::ostream& os, const boost::tuple<K, L, M>& obj)
{
    os << "{"
        << obj.template get<0>()
        << ','
        << obj.template get<1>()
        << ','
        << obj.template get<2>()
        << "}";
    return os;
}

namespace boost {
    
template <class K>
std::ostream& operator<<(std::ostream& os, const optional<K>& obj)
{
    if (obj)
    {
        os  << *obj;
    }
    else
    {
        os << '_';
    }
    return os;
}

template <class K>
std::ostream& operator<<(std::ostream& os, const scoped_ptr<K>& obj)
{
    if (obj)
    {
        os << *obj;
    }
    else
    {
        os << "       0";
    }
    return os;
}
}

#endif
