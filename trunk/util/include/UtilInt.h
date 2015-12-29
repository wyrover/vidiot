// Copyright 2013-2015 Eric Raijmakers.
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

typedef int pixel;
typedef int64_t pts;
typedef int64_t samplecount;
typedef int64_t milliseconds;
typedef int64_t microseconds;
typedef int16_t sample;

inline samplecount min(samplecount first, samplecount second)
{
    return first < second ? first : second;
}

template <typename TYPE>
TYPE min3(const TYPE& t1, const TYPE& t2, const TYPE& t3)
{
    return std::min(std::min(t1,t2),t3);
}

typedef boost::icl::interval_set<pts> PtsIntervals;
typedef boost::icl::interval_set<pixel> PixelIntervals;
typedef boost::icl::discrete_interval<pts> PtsInterval;
typedef boost::icl::discrete_interval<pixel> PixelInterval;

template <typename TYPE1, typename TYPE2>
TYPE1 removeRemainder(const TYPE2& divisor, const TYPE1& value)
{
    TYPE1 remainder = value % divisor;
    return value - remainder;
}

constexpr milliseconds sMilliSecond = 1;                ///< Basic unit of time (==1)
constexpr milliseconds sSecond = 1000 * sMilliSecond;   ///< Number of milliseconds in one second
constexpr milliseconds sMinute = 60 * sSecond;          ///< Number of milliseconds in one minute
constexpr milliseconds sHour = 60 * sMinute;            ///< Number of milliseconds in one hour
 
// From: http://www.stroustrup.com/Programming/std_lib_facilities.h                                                        
// Run-time checked narrowing cast (type conversion):
template<class R, class A> R narrow_cast(const A& a) // todo don't use for signed/unsigned casting as the assert will not be thrown (int(size_t(-19)) == -19)
{
    R r = R(a);
    ASSERT_EQUALS(A(r), a);
    return r;
}
