#include "Constants.h"

#include <math.h>

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

const int Constants::scalingPrecision = 4;
const int Constants::scalingPrecisionFactor = static_cast<int>(pow(10.0,Constants::scalingPrecision)); ///< 10^scalingPrecision
const int Constants::sMinScaling = 1;
const int Constants::sMaxScaling = 1 * scalingPrecisionFactor;

} // namespace