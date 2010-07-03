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
#include "Clip.h"
#include "TrackEventRemoveClips.h"
#include "TrackEventAddClips.h"

namespace model {

Track::Track()
:	IControl()
,   wxEvtHandler()
,   mClips()
{ 
    VAR_DEBUG(this);
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
    boost::int16_t nFrames = 0;
    BOOST_FOREACH( ClipPtr clip, mClips )
    {
        nFrames += clip->getNumberOfFrames();
    }
    return nFrames;
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
    Clips::iterator itPosition = find(mClips.begin(), mClips.end(), position);
    // NOT: ASSERT(itPosition != mClips.end()) Giving a null pointer results in mClips.end() which results in adding clips at the end
    
    mClips.splice(itPosition,clips); // See http://www.cplusplus.com/reference/stl/list/splice: clips added BEFORE position

    QueueEvent(new TrackEventAddClips(shared_from_this(), clips, position));
}

void Track::removeClips(Clips clips)
{
    Clips::iterator itBegin = find(mClips.begin(), mClips.end(), clips.front());
    ASSERT(itBegin != mClips.end())(clips.front()); // Ensure that the begin clip was found

    Clips::iterator itLast = find(itBegin, mClips.end(), clips.back());
    ASSERT(itLast != mClips.end())(clips.back()); // Ensure that the end clip was found
    
    ++itLast; // See http://www.cplusplus.com/reference/stl/list/erase: one but last is removed
    mClips.erase(itBegin,itLast);

    QueueEvent(new TrackEventRemoveClips(shared_from_this(), clips));
}

const std::list<ClipPtr>& Track::getClips()
{
    return mClips;
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
