#ifndef HELPER_MODEL_H
#define HELPER_MODEL_H

#include <boost/shared_ptr.hpp>
#include "Enums.h"

namespace model {
class IClip;
typedef boost::shared_ptr<IClip> IClipPtr;
class VideoClip;
typedef boost::shared_ptr<VideoClip> VideoClipPtr;
}

namespace test {

/// \return the given clip cast to a video clip
model::VideoClipPtr getVideoClip(model::IClipPtr clip);

/// \return the current scaling factor for the given (video) clip
/// \pre clip is a video clip
double getScalingFactor(model::IClipPtr clip);

/// \return the current scaling type for the given (video) clip
/// \pre clip is a video clip
model::VideoScaling getScaling(model::IClipPtr clip);

} // namespace

#endif // HELPER_MODEL_H