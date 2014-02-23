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

#ifndef CONSTANTS_H
#define CONSTANTS_H

namespace model {

class Constants
{
public:

    //////////////////////////////////////////////////////////////////////////
    // TIME
    //////////////////////////////////////////////////////////////////////////

    static const int sMicroseconds;             ///<Number of us in one ms (=1000)
    static const int sMilliSecond;              ///< Basic unit of time (==1)
    static const int sSecond;                   ///< Number of milliseconds in one second
    static const int sMinute;                   ///< Number of milliseconds in one minute
    static const int sHour;                     ///< Number of milliseconds in one hour

    //////////////////////////////////////////////////////////////////////////
    // LAYOUT CONSTANTS THAT ARE PERSISTED
    //////////////////////////////////////////////////////////////////////////

    static const int sDefaultTrackHeight;

    //////////////////////////////////////////////////////////////////////////
    // SCALING
    //////////////////////////////////////////////////////////////////////////

    static const int sOpacityMin;
    static const int sOpacityMax;
    static const int sScalingPrecision;       ///< Number of digits to be used maximally
    static const int sScalingPrecisionFactor; ///< 10^sScalingPrecision
    static const int sScalingPageSize;        ///< 0.1
    static const int sScalingMin;
    static const int sScalingMax;
    static const int sRotationPrecision;       ///< Number of digits to be used maximally
    static const int sRotationPrecisionFactor; ///< 10^sRotationPrecision
    static const int sRotationPageSize;        ///< 0.1
    static const int sRotationMin;
    static const int sRotationMax;
    static const int sMinVolume;
    static const int sMaxVolume;
    static const int sDefaultVolume;
};

} // namespace

#endif
