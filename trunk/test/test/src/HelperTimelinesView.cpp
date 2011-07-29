#include "HelperTimeLinesView.h"

#include "Timeline.h"
#include "TimeLinesView.h"

namespace test {

model::SequencePtr getSequence()
{
    return getTimeline().getSequence();
}

gui::timeline::Timeline& getTimeline(model::SequencePtr sequence)
{
    return gui::TimelinesView::get().getTimeline(sequence);
}

} // namespace