#include "HelperTransform.h"

#include "Details.h"
#include "DetailsClip.h"
#include "HelperTimeline.h"
#include "HelperTimelinesView.h"
#include "HelperWindow.h"
#include "Timeline.h"

#include "UtilLog.h"

namespace test {

void ResizeClip(model::IClipPtr clip, wxSize boundingbox)
{
    Click(Center(clip));
    wxWindow* current = getTimeline().getDetails().getCurrent();
    gui::timeline::DetailsClip* detailsclip = dynamic_cast<gui::timeline::DetailsClip*>(current);
    ASSERT_NONZERO(detailsclip);

}

} // namespace