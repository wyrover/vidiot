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

boost::rational<int> getScalingFactor(model::IClipPtr clip)
{
    return getVideoClip(clip)->getScalingFactor();
};

model::VideoScaling getScaling(model::IClipPtr clip)
{
    return getVideoClip(clip)->getScaling();
};

model::VideoAlignment getAlignment(model::IClipPtr clip)
{
    return getVideoClip(clip)->getAlignment();
}

wxPoint getPosition(model::IClipPtr clip)
{
    return getVideoClip(clip)->getPosition();
}

} // namespace