#ifndef FRAMERATE_H
#define FRAMERATE_H

#include <boost/rational.hpp>

namespace model {
    
typedef boost::rational<int> FrameRate;

namespace framerate {

const FrameRate s24p = FrameRate(1000, 24 * 1001);
const FrameRate s25p = FrameRate(   1, 25);
const FrameRate s30p = FrameRate(1000, 30 * 1001);

}} // namespace

#endif // FRAMERATE_H
