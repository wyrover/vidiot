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

#pragma once

template <class T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& obj)
{
    os << "{";
    bool comma{ false };
    for ( T child : obj )
    {
        if (comma)
        {
            os << ',';
        }
        else
        {
            comma = true;
        }
        os << child;
    }
    os << "}";
    return os;
}

template <class T>
std::ostream& operator<<(std::ostream& os, const std::set<T>& obj)
{
    os << "{";
    for ( T child : obj )
    {
        os << child << " ";
    }
    os << "}";
    return os;
}

template <class T, class U>
std::ostream& operator<<(std::ostream& os, const std::map<T,U>& obj)
{
    typedef typename std::map<T,U> MAPTYPE;
    os << "{";
    for ( typename MAPTYPE::value_type val : obj )
    {
        os << '(' << val.first << ',' << val.second << ")";
    }
    os << "}";
    return os;
}

template <class T, class U>
std::ostream& operator<<(std::ostream& os, const std::pair<T,U>& obj)
{
    os << "("<< obj.first << "," << obj.second << ")";
    return os;
}
