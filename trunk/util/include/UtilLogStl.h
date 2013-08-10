// Copyright 2013 Eric Raijmakers.
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

#ifndef UTIL_LOG_STL_H
#define UTIL_LOG_STL_H

template <class T>
std::ostream& operator<< (std::ostream& os, const std::list<T>& obj)
{
    os << "{";
    BOOST_FOREACH( T child, obj )
    {
        os << child << " ";
    }
    os << "}";
    return os;
}

template <class T>
std::ostream& operator<< (std::ostream& os, const std::vector<T>& obj)
{
    os << "{";
    BOOST_FOREACH( T child, obj )
    {
        os << child << " ";
    }
    os << "}";
    return os;
}

template <class T>
std::ostream& operator<< (std::ostream& os, const std::set<T>& obj)
{
    os << "{";
    BOOST_FOREACH( T child, obj )
    {
        os << child << " ";
    }
    os << "}";
    return os;
}

template <class T, class U>
std::ostream& operator<< (std::ostream& os, const std::map<T,U>& obj)
{
    typedef std::map<T,U> MAPTYPE;
    os << "{";
    BOOST_FOREACH( MAPTYPE::value_type val, obj )
    {
        os << '(' << val.first << ',' << val.second << ")";
    }
    os << "}";
    return os;
}

template <class T, class U>
std::ostream& operator<< (std::ostream& os, const std::pair<T,U>& obj)
{
    os << "("<< obj.first << "," << obj.second << ")";
    return os;
}

#endif //UTIL_LOG_STL_H