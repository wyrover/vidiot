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

#ifndef UTIL_INT_H
#define UTIL_INT_H

typedef int pixel;
typedef int64_t pts;
typedef int64_t samplecount;
typedef int16_t sample;

samplecount min(samplecount first, samplecount second);

template <typename TYPE>
TYPE min3(TYPE t1, TYPE t2, TYPE t3)
{
    return std::min(std::min(t1,t2),t3);
}

typedef boost::icl::interval_set<pts> PtsIntervals;
typedef boost::icl::interval_set<pixel> PixelIntervals;
typedef boost::icl::discrete_interval<pts> PtsInterval;
typedef boost::icl::discrete_interval<pixel> PixelInterval;

typedef boost::rational<int> rational;

int floor(rational r);

#endif //UTIL_INT_H