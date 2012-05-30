#include "HelperModel.h"

#include "Details.h"
#include "DetailsClip.h"
#include "HelperTimelinesView.h"
#include "Timeline.h"
#include "UtilLog.h"
#include "UtilLog.h"
#include "VideoClip.h"

namespace test {

void ASSERT_DETAILSCLIP(model::IClipPtr clip)
{
    wxWindow* current = getTimeline().getDetails().getCurrent();
    gui::timeline::DetailsClip* detailsclip = dynamic_cast<gui::timeline::DetailsClip*>(current);
    ASSERT_NONZERO(detailsclip);
    ASSERT_EQUALS(detailsclip->getClip(),clip);
}

} // namespace