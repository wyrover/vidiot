#ifndef UTIL_INT_H
#define UTIL_INT_H

#include <boost/cstdint.hpp>

typedef int pixel;
typedef boost::int64_t pts;
typedef int samples_t;

pts abs(pts p);

samples_t min(samples_t first, samples_t second);

#endif //UTIL_INT_H

