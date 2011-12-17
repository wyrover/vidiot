#ifndef TIMELINE_COMMAND_H
#define TIMELINE_COMMAND_H

#include <boost/shared_ptr.hpp>
#include "RootCommand.h"

namespace model {
class Sequence;
typedef boost::shared_ptr<Sequence> SequencePtr;
}
namespace gui { namespace timeline {
    class Timeline;
}}

namespace gui { namespace timeline { namespace command {

/// Abstract base for all modifications on the timeline/sequence.
class ATimelineCommand
    :   public ::command::RootCommand
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    explicit ATimelineCommand(model::SequencePtr sequence);
    virtual ~ATimelineCommand();

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    model::SequencePtr getSequence() const;
    gui::timeline::Timeline& getTimeline() const;

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    model::SequencePtr mSequence;

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<<( std::ostream& os, const ATimelineCommand& obj );
};

}}} // namespace

#endif // TIMELINE_COMMAND_H