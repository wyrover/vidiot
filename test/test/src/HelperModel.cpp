#include "HelperModel.h"

#include "UtilLog.h"
#include "VideoClip.h"

namespace test {

model::VideoClipPtr getVideoClip(model::IClipPtr clip)
{
    model::VideoClipPtr videoclip = boost::dynamic_pointer_cast<model::VideoClip>(clip);
    ASSERT(videoclip);
    return videoclip;
}

double getScalingFactor(model::IClipPtr clip)
{
    return getVideoClip(clip)->getScalingFactor();
};

model::VideoScaling getScaling(model::IClipPtr clip)
{
    return getVideoClip(clip)->getScaling();
};

} // namespace