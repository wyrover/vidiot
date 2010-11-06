#include "Track.h"

#include <algorithm>
#include <boost/make_shared.hpp>
#include <boost/foreach.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/base_object.hpp>
#include "UtilLog.h"
#include "AProjectViewNode.h"
#include "UtilLogStl.h"
#include "Clip.h"

namespace model {

static int sDefaultTrackHeight = 50;

DEFINE_EVENT(EVENT_ADD_CLIPS,      EventAddClips,      MoveParameter);
DEFINE_EVENT(EVENT_REMOVE_CLIPS,   EventRemoveClips,   MoveParameter);

Track::Track()
:	IControl()
,   wxEvtHandler()
,   mClips()
,   mHeight(sDefaultTrackHeight)
{ 
    VAR_DEBUG(this);
}

Track::Track(const Track& other)
:	IControl()
,   wxEvtHandler()
,   mClips()
,   mHeight(other.mHeight)
{
    VAR_DEBUG(this);
    ASSERT(false);// If this is ever used, test the clips in combination with the shared_from_this() in addClips below.

    Clips clonedClips;
    BOOST_FOREACH(ClipPtr clip, other.mClips)
    {
        clonedClips.push_back(make_cloned<Clip>(clip));
    }
    addClips(clonedClips,ClipPtr());
}

Track* Track::clone()
{ 
    return new Track(static_cast<const Track&>(*this)); 
}

Track::~Track()
{
    VAR_DEBUG(this);
}

//////////////////////////////////////////////////////////////////////////
// ICONTROL
//////////////////////////////////////////////////////////////////////////

boost::int64_t Track::getNumberOfFrames()
{
    /** @todo return rightPts of last clip? */
    boost::int16_t nFrames = 0;
    BOOST_FOREACH( ClipPtr clip, mClips )
    {
        nFrames += clip->getNumberOfFrames();
    }
    return nFrames;
}


/** @todo replace with clip->getrightpts? */
boost::int64_t Track::getStartFrameNumber(ClipPtr clip) const
{
    boost::int16_t n = 0;
    BOOST_FOREACH( ClipPtr usedclip, mClips )
    {
        if (usedclip == clip)
        {
            return n;
        }
        n += usedclip->getNumberOfFrames();
    }
    FATAL("Clip is not a part of this track.");
    return 0;
}

void Track::moveTo(boost::int64_t position)
{
    VAR_DEBUG(this)(position);

    // mItClips may become mClips.end() signaling that this is beyond the last clip

    mItClips = mClips.begin();
    if (mItClips == mClips.end())
    {
        // No clips in track
        return;
    }

    boost::int64_t lastFrame = (*mItClips)->getNumberOfFrames(); // There is at least one clip due to the check above
    boost::int64_t firstFrame = 0;

    while (lastFrame < position)
    {
        firstFrame += (*mItClips)->getNumberOfFrames();
        ++mItClips;

        if (mItClips == mClips.end())
        {
            // Position beyond track size.
            return;
        }

        lastFrame += (*mItClips)->getNumberOfFrames();
    }

    ASSERT(position <= lastFrame)(position)(lastFrame);
    (*mItClips)->moveTo(position - firstFrame);// - 1); // -1: Counting starts at 0
}

//////////////////////////////////////////////////////////////////////////
// HANDLING CLIPS
//////////////////////////////////////////////////////////////////////////

void Track::addClips(Clips clips, ClipPtr position)
{
    boost::int64_t pts = 0;  // Default initialization for the case position is not initialized (add at end)

    if (position)
    {
        pts = position->getRightPts();
    }
    else
    {
        if (!mClips.empty())
        {
            // Position at end
            pts = (*mClips.rbegin())->getRightPts();
        }
    }

    BOOST_FOREACH( ClipPtr clip, clips )
    {
        clip->setTrack(shared_from_this(), pts);
        pts += clip->getNumberOfFrames();
    }

    Clips::iterator itPosition = find(mClips.begin(), mClips.end(), position);
    // NOT: ASSERT(itPosition != mClips.end()) Giving a null pointer results in mClips.end() which results in adding clips at the end

    // See http://www.cplusplus.com/reference/stl/list/splice:
    // clips will be removed from this list. Hence, a copy is made,
    // before doing the splice call.
    MoveParameter move(shared_from_this(), position, clips, model::TrackPtr(), model::ClipPtr(), model::Clips());

    mClips.splice(itPosition,clips); // See http://www.cplusplus.com/reference/stl/list/splice: clips added BEFORE position
    VAR_DEBUG(mClips);

    QueueEvent(new model::EventAddClips(move));

    /** @todo combine consecutive empty clips */

    /** @todo ensure that all tracks keep on having same length by adding/removing empty at end */
    /** @todo use moveTo to reposition to the 'same position' as before the change. */
}

void Track::removeClips(Clips clips)
{
    BOOST_FOREACH( ClipPtr clip, clips )
    {
        clip->setTrack(TrackPtr(), 0);
    }

    Clips::iterator itBegin = find(mClips.begin(), mClips.end(), clips.front());
    ASSERT(itBegin != mClips.end())(clips.front()); // Ensure that the begin clip was found

    Clips::iterator itLast = find(itBegin, mClips.end(), clips.back());
    ASSERT(itLast != mClips.end())(clips.back()); // Ensure that the end clip was found
    
    ++itLast; // See http://www.cplusplus.com/reference/stl/list/erase: one but last is removed
    mClips.erase(itBegin,itLast);
    VAR_DEBUG(mClips);

    MoveParameter move(model::TrackPtr(), model::ClipPtr(), model::Clips(), shared_from_this(), model::ClipPtr(), clips);
    QueueEvent(new model::EventRemoveClips(move));

    /** @todo combine consecutive empty clips */
    /** @todo ensure that all tracks keep on having same length by adding/removing empty at end */
    /** @todo use moveTo to reposition to the 'same position' as before the change. */
}

const std::list<ClipPtr>& Track::getClips()
{
    return mClips;
}

ClipPtr Track::getClip(boost::int64_t pts)
{
    boost::int64_t left = 0;
    boost::int64_t right = left;
    BOOST_FOREACH( ClipPtr clip, mClips )
    {
        boost::int64_t length = clip->getNumberOfFrames();
        right += length;
        if (pts >= left && pts <= right)
        {
            return clip;
        }
        left += length;
    }
    return ClipPtr();
}

ClipPtr Track::getNextClip(ClipPtr clip)
{
    Clips::iterator it = find(mClips.begin(),mClips.end(),clip);
    ASSERT(it != mClips.end());
    ++it;
    if (it == mClips.end())
    {
        return ClipPtr();
    }
    return *it;
}

ClipPtr Track::getPreviousClip(ClipPtr clip)
{
    Clips::iterator it = find(mClips.begin(),mClips.end(),clip);
    ASSERT(it != mClips.end());
    if (it == mClips.begin())
    {
        return ClipPtr();
    }
    --it;
    return *it; 
}

//////////////////////////////////////////////////////////////////////////
// GET & SET
//////////////////////////////////////////////////////////////////////////

int Track::getHeight()
{
    return mHeight;
}

//////////////////////////////////////////////////////////////////////////
// ITERATION
//////////////////////////////////////////////////////////////////////////

bool Track::iterate_hasClip() 
{
    return (mItClips != mClips.end());
}

ClipPtr Track::iterate_getClip()
{
    ASSERT(iterate_hasClip());
    return *mItClips;
}

void Track::iterate_nextClip()
{
    ASSERT(iterate_hasClip());
    mItClips++;
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION 
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void Track::save(Archive & ar, const unsigned int version) const
{
    ar & boost::serialization::base_object<IControl>(*this);
    ar & mClips;
}
template<class Archive>
void Track::load(Archive & ar, const unsigned int version)
{
    ar & boost::serialization::base_object<IControl>(*this);
    ar & mClips;
    if (Archive::is_loading::value)
    {
        mItClips = mClips.begin();
    }
}
template void Track::save<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion) const;
template void Track::load<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);

} //namespace
