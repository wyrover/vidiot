#include "HelperTransform.h"

#include "HelperWindow.h"
#include "UtilLog.h"
#include "Timeline.h"
#include "HelperTimeline.h"
#include "HelperTimelinesView.h"
#include "DetailsClip.h"
#include "Details.h"

namespace test {

void ResizeClip(model::IClipPtr clip, wxSize boundingbox)
{
    Click(Center(clip));
    wxWindow* current = getTimeline().getDetails().getCurrent();
    gui::timeline::DetailsClip* detailsclip = dynamic_cast<gui::timeline::DetailsClip*>(current);
    ASSERT_NONZERO(detailsclip);

}

} // namespace