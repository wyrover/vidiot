#ifndef UTIL_INT_H
#define UTIL_INT_H

#include <boost/cstdint.hpp>
#include <boost/icl/interval_set.hpp>

typedef int pixel;
typedef boost::int64_t pts;
typedef int samplecount;
typedef boost::int16_t sample;

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

#endif //UTIL_INT_H