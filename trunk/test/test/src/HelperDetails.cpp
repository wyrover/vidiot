#include "HelperModel.h"

#include "Constants.h"
#include "Convert.h"
#include "Details.h"
#include "DetailsClip.h"
#include "HelperTimelinesView.h"
#include "Timeline.h"
#include "UtilLog.h"
#include "UtilLogWxwidgets.h"
#include "VideoClip.h"

namespace test {

gui::timeline::DetailsClip* DetailsClipView()
{
    wxWindow* current = getTimeline().getDetails().getCurrent();
    gui::timeline::DetailsClip* detailsclip = dynamic_cast<gui::timeline::DetailsClip*>(current);
    ASSERT_NONZERO(detailsclip);
    return detailsclip;
}

void ASSERT_DETAILSCLIP(model::IClipPtr clip)
{
    ASSERT_EQUALS(DetailsClipView()->getClip(),clip);
}

void ASSERT_CLIPPROPERTIES(model::IClipPtr clip, model::VideoScaling scaling, int scalingdigits, model::VideoAlignment alignment, wxPoint position)
{
    model::VideoClipPtr videoclip = getVideoClip(clip);
    ASSERT_DETAILSCLIP(clip);
    ASSERT_EQUALS(DetailsClipView()->getScalingSlider()->GetValue(),scalingdigits);
    ASSERT_EQUALS(model::Convert::factorToDigits(DetailsClipView()->getScalingSpin()->GetValue(),model::Constants::scalingPrecision),scalingdigits);
    ASSERT_EQUALS(DetailsClipView()->getPositionXSlider()->GetValue(),position.x);
    ASSERT_EQUALS(DetailsClipView()->getPositionXSpin()->GetValue(),position.x);
    ASSERT_EQUALS(DetailsClipView()->getPositionYSlider()->GetValue(),position.y);
    ASSERT_EQUALS(DetailsClipView()->getPositionYSpin()->GetValue(),position.y);
    ASSERT_EQUALS(videoclip->getScaling(),scaling);
    ASSERT_EQUALS(videoclip->getScalingDigits(),scalingdigits);
    ASSERT_EQUALS(videoclip->getAlignment(),alignment);
    ASSERT_EQUALS(videoclip->getPosition(),position);
};

} // namespace