#ifndef HELPER_TIMELINESVIEW_H
#define HELPER_TIMELINESVIEW_H

#include <boost/shared_ptr.hpp>

namespace model {
class Sequence;
typedef boost::shared_ptr<Sequence> SequencePtr;
}

namespace gui { namespace timeline {
class Timeline;
}}

namespace test {

/// \return Currently active seqence (the sequence for which the timeline is the active notebook page)
model::SequencePtr getSequence();

/// Return the opened timeline for a sequence
/// \param sequence if this equals 0 then the active timeline is returned
gui::timeline::Timeline& getTimeline(model::SequencePtr sequence = model::SequencePtr());

} // namespace

#endif // HELPER_TIMELINESVIEW_H