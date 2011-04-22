#ifndef UTIL_INT_H
#define UTIL_INT_H

#include <boost/cstdint.hpp>

typedef int pixel;
typedef boost::int64_t pts;
typedef int samplecount;
typedef boost::int16_t sample;

pts abs(pts p);

samplecount min(samplecount first, samplecount second);

#endif //UTIL_INT_H

