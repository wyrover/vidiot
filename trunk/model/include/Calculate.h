#ifndef MODEL_CALCULATE_H
#define MODEL_CALCULATE_H

#include <list>
#include <boost/shared_ptr.hpp>
#include "UtilInt.h"

namespace model {

class IClip;
typedef boost::shared_ptr<IClip> IClipPtr;
typedef std::list<IClipPtr> IClips;

namespace calculate {

pts combinedLength(const IClips& clips);

}} // namespace

#endif // MODEL_CALCULATE_H