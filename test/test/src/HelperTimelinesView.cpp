#include "HelperTimeLinesView.h"

#include "Render.h"
#include "Sequence.h"
#include "Timeline.h"
#include "TimeLinesView.h"

namespace test {

model::SequencePtr getSequence()
{
    return getTimeline().getSequence();
}

model::render::RenderPtr getCurrentRenderSettings()
{
    return make_cloned<model::render::Render>(getSequence()->getRender());
}

gui::timeline::Timeline& getTimeline(model::SequencePtr sequence)
{
    return gui::TimelinesView::get().getTimeline(sequence);
}

} // namespace