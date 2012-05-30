#ifndef HELPER_DETAILS_H
#define HELPER_DETAILS_H

#include <wx/gdicmn.h>
#include <boost/shared_ptr.hpp>
#include "Enums.h"

namespace model {
class IClip;
typedef boost::shared_ptr<IClip> IClipPtr;
class VideoClip;
typedef boost::shared_ptr<VideoClip> VideoClipPtr;
}

namespace test {

void ASSERT_DETAILSCLIP(model::IClipPtr clip);

} // namespace

#endif // HELPER_DETAILS_H