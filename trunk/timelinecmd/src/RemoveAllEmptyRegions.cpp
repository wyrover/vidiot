#include "RemoveAllEmptyRegions.h"

#include "UtilLog.h"
#include "AudioTrack.h"
#include "Timeline.h"
#include "Sequence.h"
#include "EmptyClip.h"
#include "SequenceEvent.h"

namespace gui { namespace timeline { namespace command {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

RemoveAllEmptyRegions::RemoveAllEmptyRegions(model::SequencePtr sequence)
    :   AClipEdit(sequence)
{
    VAR_INFO(this);
    mCommandName = _("Remove all empty regions");

    auto makeInterval = [] (pts a, pts b) -> PtsInterval
    {
        VAR_DEBUG(a)(b);
        return PtsInterval(std::min(a,b),std::max(a,b));
    };

    mRemoved.clear();
    mRemoved.insert(makeInterval(0,getSequence()->getLength()));
    BOOST_FOREACH( model::TrackPtr track, getTimeline().getSequence()->getTracks() )
    {
        BOOST_FOREACH( model::IClipPtr clip, track->getClips() )
        {
            if (!clip->isA<model::EmptyClip>())
            {
                mRemoved -= makeInterval(clip->getLeftPts(), clip->getRightPts());
            }
        }
    }
    VAR_INFO(mRemoved);
}

RemoveAllEmptyRegions::~RemoveAllEmptyRegions()
{
}

//////////////////////////////////////////////////////////////////////////
// ACLIPEDIT INTERFACE
//////////////////////////////////////////////////////////////////////////

void RemoveAllEmptyRegions::initialize()
{
    showAnimation();

    getTimeline().beginTransaction();
    Revert();

    std::set< model::IClips > removedInAllTracks = splitTracksAndFindClipsToBeRemoved(mRemoved);
    BOOST_FOREACH( model::IClips remove, removedInAllTracks )
    {
        removeClips(remove);
    }
    getTimeline().endTransaction();

}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void RemoveAllEmptyRegions::showAnimation()
{
    std::set< model::IClips > removedInAllTracks = splitTracksAndFindClipsToBeRemoved(mRemoved);

    model::IClips mEmpties;
    BOOST_FOREACH( model::IClips remove, removedInAllTracks )
    {
        mEmpties.push_back(replaceWithEmpty(remove));
    }

    wxSafeYield(); // Show update progress, but do not allow user input
    boost::this_thread::sleep(boost::posix_time::milliseconds(200));

    animatedTrimEmpty(mEmpties);
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<( std::ostream& os, const RemoveAllEmptyRegions& obj )
{
    os << static_cast<const AClipEdit&>(obj) << '|' << obj.mRemoved;;
    return os;
}

}}} // namespace