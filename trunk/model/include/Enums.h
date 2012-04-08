#ifndef ENUMS_H
#define ENUMS_H

#include "UtilEnum.h"

namespace model {

/// VideoScalingFitAll      : Scale such that all of the video fits in the target area.
/// VideoScalingFitToFill   : Scale such that one one fo the sizes fits exactly in the target area. In the other direction, the video either fits exactly or is larger.
/// VideoScalingNone        : Keep original size
/// VideoScalingCustom      : User specified scaling factor
DECLAREENUM(VideoScaling, \
            VideoScalingFitAll, \
            VideoScalingFitToFill, \
            VideoScalingNone, \
            VideoScalingCustom);

DECLAREENUM(VideoAlignment, \
            VideoAlignmentCenter, \
            VideoAlignmentCustom);

}

#endif // ENUMS_H