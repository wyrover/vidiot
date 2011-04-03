#include "Track.h"

#include <wx/intl.h> 
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
#include "Constants.h"
#include "Clip.h"
#include "UtilList.h"
#include "EmptyClip.h"

namespace model {

DEFINE_EVENT(EVENT_ADD_CLIPS,      EventAddClips,      MoveParameter);
DEFINE_EVENT(EVENT_REMOVE_CLIPS,   EventRemoveClips,   MoveParameter);
DEFINE_EVENT(EVENT_HEIGHT_CHANGED, EventHeightChanged, int);

Track::Track()
:	IControl()
,   wxEvtHandler()
,   mClips()
,   mItClips(mClips.end())
,   mHeight(Constants::sDefaultTrackHeight)
,   mIndex(0)
{ 
    VAR_DEBUG(this);
}

Track::Track(const Track& other)
:	IControl()
,   wxEvtHandler()
,   mClips()
,   mItClips(mClips.end())
,   mHeight(other.mHeight)
,   mIndex(0)
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
    // This (removeClips) is needed to let all observer classes know that 
    // the clips are removed from the tracks. This in turn ensures that 
    // all owners of shared_ptr to these clips can remove the shared_ptr 
    // use, resulting in the actual destruction of the clips.
    removeClips(mClips);
}

//////////////////////////////////////////////////////////////////////////
// ICONTROL
//////////////////////////////////////////////////////////////////////////

pts Track::getLength()
{
    /** @todo return rightPts of last clip? */
    boost::int16_t nFrames = 0;
    BOOST_FOREACH( ClipPtr clip, mClips )
    {
        nFrames += clip->getLength();
    }
    return nFrames;
}


void Track::moveTo(pts position)
{
    VAR_DEBUG(this)(position);

    // mItClips may become mClips.end() signaling that this is beyond the last clip

    mItClips = mClips.begin();
    if (mItClips == mClips.end())
    {
        // No clips in track
        return;
    }

    pts lastFrame = (*mItClips)->getLength(); // There is at least one clip due to the check above
    pts firstFrame = 0;

    while (lastFrame < position)
    {
        firstFrame += (*mItClips)->getLength();
        ++mItClips;

        if (mItClips == mClips.end())
        {
            // Position beyond track size.
            return;
        }

        lastFrame += (*mItClips)->getLength();
    }

    ASSERT(position <= lastFrame)(position)(lastFrame);
    (*mItClips)->moveTo(position - firstFrame);// - 1); // -1: Counting starts at 0
}

wxString Track::getDescription() const
{
    static wxString track = _("Track");
    return wxString::Format(wxT("%s %02d"), track, mIndex);
}

void Track::clean()
{
    VAR_DEBUG(this);
    BOOST_FOREACH(ClipPtr clip, mClips)
    {
        clip->clean();
    }
    mItClips = mClips.end();
}

//////////////////////////////////////////////////////////////////////////
// HANDLING CLIPS
//////////////////////////////////////////////////////////////////////////

/// \TODO: Make Track::execute(MoveParameter param), then the command just passes the info on.
/// 		  Furthermore, make MoveParameter a separate file (MoveClip.*)
/// 		  Finally give that class a 'clone_invert' method that returns a cloned and inverted instance.

void Track::addClips(Clips clips, ClipPtr position)
{
    VAR_DEBUG(*this)(position)(clips);
    UtilList<ClipPtr>(mClips).addElements(clips,position);

	updateClips();

    moveTo(0); // Required since the list iterator has become invalid.

    // ProcessEvent is used. Model events must be processed synchronously to avoid inconsistent states in
    // the receivers of these events (typically, the view classes in the timeline).
    // Example:
    // 1. Add clip
    // 2. Remove clip again
    // 3. Event of addition is received a bit later. Here the added clip is no longer part of the track. ERROR.
    ProcessEvent(EventAddClips(MoveParameter(shared_from_this(), position, clips, model::TrackPtr(), model::ClipPtr(), model::Clips()))); // Must be handled immediately
}

void Track::removeClips(Clips clips)
{
    VAR_DEBUG(*this)(clips);
	BOOST_FOREACH( ClipPtr clip, clips )
	{
        clip->clean();
		clip->setTrack(TrackPtr(), 0);
	}

	ClipPtr position = UtilList<ClipPtr>(mClips).removeElements(clips);

	updateClips();

    moveTo(0); // Required since the list iterator has become invalid.

    // ProcessEvent is used. Model events must be processed synchronously to avoid inconsistent states in
    // the receivers of these events (typically, the view classes in the timeline).
    // Example:
    // 1. Add clip
    // 2. Remove clip again
    // 3. Event of addition is received a bit later. Here the added clip is no longer part of the track. ERROR.
	ProcessEvent(EventRemoveClips(MoveParameter(model::TrackPtr(), model::ClipPtr(), model::Clips(), shared_from_this(), position, clips))); // Must be handled immediately
}

const std::list<ClipPtr>& Track::getClips()
{
    return mClips;
}

ClipPtr Track::getClip(pts position)
{
	pts left = 0;
    pts right = left;
    BOOST_FOREACH( ClipPtr clip, mClips )
    {
        pts length = clip->getLength();
        right += length;
        if (position >= left && position < right) // < right: clip->getrightpts == nextclip->getleftpts
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

pts Track::getLeftEmptyArea(model::ClipPtr clip)
{
    ASSERT(UtilList<ClipPtr>(mClips).hasElement(clip));
    pts leftmost = clip->getLeftPts();
    model::ClipPtr previous = getPreviousClip(clip);
    while (previous && previous->isA<model::EmptyClip>())
    {
        leftmost = previous->getLeftPts();
        previous = getPreviousClip(previous);
    }
    return leftmost - clip->getLeftPts();
}

pts Track::getRightEmptyArea(model::ClipPtr clip)
{
    ASSERT(UtilList<ClipPtr>(mClips).hasElement(clip));
    pts rightmost = clip->getRightPts();
    model::ClipPtr next = getNextClip(clip);
    while (next && next->isA<model::EmptyClip>())
    {
        rightmost = next->getRightPts();
        next = getNextClip(next);
    }
    return rightmost - clip->getRightPts();
}

//////////////////////////////////////////////////////////////////////////
// GET & SET
//////////////////////////////////////////////////////////////////////////

int Track::getHeight() const
{
    return mHeight;
}

void Track::setHeight(int height)
{
    mHeight = height;
    ProcessEvent(model::EventHeightChanged(height));
}

int Track::getIndex() const
{
    return mIndex;
}

void Track::setIndex(int index)
{
    mIndex = index;
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
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void Track::updateClips()
{
	pts position = 0;
    int index = 0;
	BOOST_FOREACH( ClipPtr clip, mClips )
	{
		clip->setTrack(shared_from_this(), position, index);
		position += clip->getLength();
        index++;
	}
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<( std::ostream& os, const Track& obj )
{
    os << &obj << '|' << obj.mIndex << '|' << obj.mHeight << '|' << obj.mClips;
    return os;
}

std::ostream& operator<<( std::ostream& os, const MoveParameter& obj )
{
    os << obj.removeTrack << '|' << obj.removePosition << '|' << obj.removeClips << '|' << obj.addTrack << '|' << obj.addPosition << '|' << obj.addClips; 
    return os;
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION 
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void Track::save(Archive & ar, const unsigned int version) const
{
    ar & boost::serialization::base_object<IControl>(*this);
    ar & mIndex;
    ar & mClips;
}
template<class Archive>
void Track::load(Archive & ar, const unsigned int version)
{
    ar & boost::serialization::base_object<IControl>(*this);
    ar & mIndex;
    ar & mClips;
    if (Archive::is_loading::value)
    {
        mItClips = mClips.begin();
        updateClips();
    }
}
template void Track::save<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion) const;
template void Track::load<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);

} //namespace
