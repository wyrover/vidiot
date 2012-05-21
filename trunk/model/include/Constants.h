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

    static const int scalingPrecision;       ///< Number of digits to be used maximally
    static const int scalingPrecisionFactor; ///< 10^scalingPrecision
    static const int scalingPageSize;        ///< 0.1
    static const int sMinScaling;
    static const int sMaxScaling;
};

} // namespace

#endif // CONSTANTS_H