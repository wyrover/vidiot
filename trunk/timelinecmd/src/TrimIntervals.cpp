#include "TrimIntervals.h"

#include "Cursor.h"
#include "EmptyClip.h"
#include "IClip.h"
#include "Intervals.h"
#include "Application.h"
#include "Sequence.h"
#include "Timeline.h"
#include "Timeline.h"
#include "Track.h"
#include "Transition.h"
#include "UtilLog.h"
#include "UtilLogWxwidgets.h"
#include "Zoom.h"

namespace gui { namespace timeline { namespace command {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

TrimIntervals::TrimIntervals(model::SequencePtr sequence, PtsIntervals intervals, bool deleteMarked)
    :   AClipEdit(sequence)
    ,   mIntervals(getTimeline().getIntervals().get())
    ,   mRemoved(getTimeline().getIntervals().get())
    ,   mDeleteMarked(deleteMarked)
{
    VAR_INFO(this)(mIntervals)(mDeleteMarked);
    mCommandName = mDeleteMarked ? _("Remove marked area") : _("Remove unmarked area");

    if (!mDeleteMarked)
    {
        PtsIntervals unmarked;
        unmarked += PtsInterval(0,getSequence()->getLength());
        unmarked -= mIntervals;
        mRemoved = unmarked;
    }
}

TrimIntervals::~TrimIntervals()
{
}

//////////////////////////////////////////////////////////////////////////
// ACLIPEDIT INTERFACE
//////////////////////////////////////////////////////////////////////////

void TrimIntervals::initialize()
{
    showAnimation();

    getTimeline().beginTransaction();
    Revert();

    getTimeline().getIntervals().removeAll();
    std::set< model::IClips > removedInAllTracks = splitTracksAndFindClipsToBeRemoved();
    BOOST_FOREACH( model::IClips remove, removedInAllTracks )
    {
        removeClips(remove);
    }
    getTimeline().endTransaction();
}

void TrimIntervals::doExtra()
{
    LOG_INFO;
    getTimeline().getIntervals().removeAll();
}

void TrimIntervals::undoExtra()
{
    getTimeline().getIntervals().set(mIntervals);
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

std::set< model::IClips > TrimIntervals::splitTracksAndFindClipsToBeRemoved()
{
    std::set< model::IClips > result;
    BOOST_FOREACH( PtsInterval interval, mRemoved )
    {
        pts first = interval.lower();
        pts last = interval.upper() - 1;
        ASSERT_LESS_THAN(first,last);
        BOOST_FOREACH( model::TrackPtr track, getSequence()->getTracks() )
        {
            split(track, first);
            split(track, last);
            model::IClips removedInTrack;
            BOOST_FOREACH( model::IClipPtr clip, track->getClips() )
            {
                if (clip->getLeftPts() >= last)
                {
                    break;
                }
                if (clip->getLeftPts() >= first)
                {
                    removedInTrack.push_back(clip);
                }
            }
            result.insert(removedInTrack);
        }
    }
    return result;
}

void TrimIntervals::showAnimation()
{
    std::set< model::IClips > removedInAllTracks = splitTracksAndFindClipsToBeRemoved();

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

std::ostream& operator<<( std::ostream& os, const TrimIntervals& obj )
{
    os << static_cast<const AClipEdit&>(obj) << '|' << obj.mDeleteMarked << '|' << obj.mIntervals << '|' << obj.mRemoved;
    return os;
}
}}} // namespace