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

#include "Constants.h"

namespace model {

//////////////////////////////////////////////////////////////////////////
// TIME
//////////////////////////////////////////////////////////////////////////

const int Constants::sMicroseconds  = 1000;
const int Constants::sMilliSecond   = 1;
const int Constants::sSecond        = 1000  * sMilliSecond;
const int Constants::sMinute        = 60    * sSecond;
const int Constants::sHour          = 60    * sMinute;

//////////////////////////////////////////////////////////////////////////
// LAYOUT CONSTANTS THAT ARE PERSISTED
//////////////////////////////////////////////////////////////////////////

const int Constants::sDefaultTrackHeight = 50;

//////////////////////////////////////////////////////////////////////////
// SCALING
//////////////////////////////////////////////////////////////////////////

const int Constants::sMinOpacity =  wxIMAGE_ALPHA_TRANSPARENT;
const int Constants::sMaxOpacity =  wxIMAGE_ALPHA_OPAQUE;
const int Constants::scalingPrecision = 4;
const int Constants::scalingPrecisionFactor = static_cast<int>(pow(10.0,Constants::scalingPrecision)); ///< 10^scalingPrecision
const int Constants::scalingPageSize = Constants::scalingPrecisionFactor / 10; // 0.1
const int Constants::sMinScaling = 1;
const int Constants::sMaxScaling = 10 * scalingPrecisionFactor;
const int Constants::sMinVolume = 1;
const int Constants::sMaxVolume = 200;
const int Constants::sDefaultVolume = 100;

} // namespace