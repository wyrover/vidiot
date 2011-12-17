#include "ATimelineCommand.h"

#include "TimeLinesView.h"

namespace gui { namespace timeline { namespace command {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

ATimelineCommand::ATimelineCommand(model::SequencePtr sequence)
    :   RootCommand()
    ,   mSequence(sequence)
{
}

ATimelineCommand::~ATimelineCommand()
{
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

model::SequencePtr ATimelineCommand::getSequence() const
{
    return mSequence;
}

gui::timeline::Timeline& ATimelineCommand::getTimeline() const
{
    return gui::TimelinesView::get().getTimeline(mSequence);
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<( std::ostream& os, const ATimelineCommand& obj )
{
    os << &obj << '|' << typeid(obj).name();
    return os;
}

}}} // namespace