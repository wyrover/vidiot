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

#include "UtilInt.h"

samplecount min(samplecount first, samplecount second)
{
    return first < second ? first : second;
}

int floor(rational r)
{
    return static_cast<int>(floor(boost::rational_cast<double>(r)));
}

int64_t floor64(rational64 r)
{
    return static_cast<int64_t>(boost::rational_cast<double>(r));
}