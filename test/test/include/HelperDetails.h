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

namespace gui { namespace timeline {
    class DetailsClip;
}}

namespace test {

gui::timeline::DetailsClip* DetailsClipView();

void ASSERT_DETAILSCLIP(model::IClipPtr clip);

/// Check the properties of the current details view and the given clip
/// \pre The given clip must be selected and shown in the details view
void ASSERT_CLIPPROPERTIES(model::IClipPtr clip, model::VideoScaling scaling, boost::rational<int> scalingfactor, model::VideoAlignment alignment, wxPoint position);

} // namespace

#endif // HELPER_DETAILS_H