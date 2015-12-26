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

typedef boost::rational<int32_t> rational32;
typedef boost::rational<int64_t> rational64;

inline int32_t floor(boost::rational<int32_t> r)
{
    return static_cast<int32_t>(boost::rational_cast<double>(r));
}

inline int64_t floor(boost::rational<int64_t> r)
{
    return static_cast<int64_t>(boost::rational_cast<double>(r));

}