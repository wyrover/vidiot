#include "Calculate.h"

#include <boost/foreach.hpp>
#include "IClip.h"

namespace model { namespace calculate {

pts combinedLength(const IClips& clips)
{
    int length = 0;
    BOOST_FOREACH( IClipPtr clip, clips )
    {
        length += clip->getLength();
    }
    return length;
}

}} //namespace