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
#include "Transition.h"
#include "UtilList.h"
#include "EmptyClip.h"

namespace model {

DEFINE_EVENT(EVENT_ADD_CLIPS,           EventAddClips,          MoveParameter);
DEFINE_EVENT(EVENT_REMOVE_CLIPS,        EventRemoveClips,       MoveParameter);
DEFINE_EVENT(EVENT_HEIGHT_CHANGED,      EventHeightChanged,     int);

Track::Track()
:	IControl()
,   wxEvtHandler()
,   mClips()
,   mItCurrent()
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
,   mItCurrent()
,   mItClips(mClips.end())
,   mHeight(other.mHeight)
,   mIndex(0)
{
    VAR_DEBUG(this);
    ASSERT(false);// If this is ever used, test the clips in combination with the shared_from_this() in addClips below.

    IClips clonedClips;
    BOOST_FOREACH(IClipPtr clip, other.mClips)
    {
        clonedClips.push_back(make_cloned<IClip>(clip));
    }
    addClips(clonedClips,IClipPtr());
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

pts Track::getLength()
{
// todo    return getCombinedLength(mClips);
    /** @todo return rightPts of last clip? */
    boost::int16_t nFrames = 0;
    BOOST_FOREACH( IClipPtr clip, mClips )
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
    mItCurrent = *mItClips;
    mItCurrent->moveTo(position - firstFrame);// - 1); // -1: Counting starts at 0
}

wxString Track::getDescription() const
{
    static wxString track = _("Track");
    return wxString::Format(wxT("%s %02d"), track, mIndex);
}

void Track::clean()
{
    VAR_DEBUG(this);
    BOOST_FOREACH(IClipPtr clip, mClips)
    {
        clip->clean();
    }
    mItCurrent.reset();
    mItClips = mClips.end();
}

//////////////////////////////////////////////////////////////////////////
// HANDLING CLIPS
//////////////////////////////////////////////////////////////////////////

/// \TODO: Make Track::execute(MoveParameter param), then the command just passes the info on.
/// 		  Furthermore, make MoveParameter a separate file (MoveClip.*)
/// 		  Finally give that class a 'clone_invert' method that returns a cloned and inverted instance.

void Track::addClips(IClips clips, IClipPtr position)
{
    VAR_DEBUG(*this)(position)(clips);

    UtilList<IClipPtr>(mClips).addElements(clips,position);

	updateClips();

    moveTo(0); // Required since the iteration has become invalid.

    // ProcessEvent is used. Model events must be processed synchronously to avoid inconsistent states in
    // the receivers of these events (typically, the view classes in the timeline).
    // Example:
    // 1. Add clip
    // 2. Remove clip again
    // 3. Event of addition is received a bit later. Here the added clip is no longer part of the track. ERROR.
    ProcessEvent(EventAddClips(MoveParameter(shared_from_this(), position, clips, TrackPtr(), IClipPtr(), IClips()))); // Must be handled immediately
}

void Track::removeClips(IClips clips)
{
    VAR_DEBUG(*this)(clips);

    BOOST_FOREACH( IClipPtr clip, clips )
    {
        clip->clean();
        clip->setTrack(TrackPtr(), 0);
    }

	IClipPtr position = UtilList<IClipPtr>(mClips).removeElements(clips);

    updateClips();

    moveTo(0); // Required since the iteration has become invalid.

    // ProcessEvent is used. Model events must be processed synchronously to avoid inconsistent states in
    // the receivers of these events (typically, the view classes in the timeline).
    // Example:
    // 1. Add clip
    // 2. Remove clip again
    // 3. Event of addition is received a bit later. Here the added clip is no longer part of the track. ERROR.
	ProcessEvent(EventRemoveClips(MoveParameter(TrackPtr(), IClipPtr(), IClips(), shared_from_this(), position, clips))); // Must be handled immediately
}

const IClips& Track::getClips()
{
    return mClips;
}

IClipPtr Track::getClip(pts position)
{
	pts left = 0;
    pts right = left;
    IClipPtr found;
    BOOST_FOREACH( IClipPtr clip, mClips )
    {
        pts length = clip->getLength();
        right += length;
        if (position >= left && position < right) // < right: clip->getrightpts == nextclip->getleftpts
        {
            found = clip;
            break;
        }
        left += length;
    }
    if (found)
    {
        // todo more efficient
        IClipPtr next = getNextClip(found);
        if (next && next->isA<Transition>() && next->getLeftPts() <= position && next->getRightPts() > position)
        {
            return next;
        }
        IClipPtr previous = getPreviousClip(found);
        if (previous && previous->isA<Transition>() && previous->getLeftPts() <= position && previous->getRightPts() > position)
        {
            return previous;
        }
        return found;
    }
    return IClipPtr();
}

IClipPtr Track::getNextClip(IClipPtr clip)
{
    IClips::iterator it = find(mClips.begin(),mClips.end(),clip);
    ASSERT(it != mClips.end());
    ++it;
    if (it == mClips.end())
    {
        return IClipPtr();
    }
    return *it;
}

IClipPtr Track::getPreviousClip(IClipPtr clip)
{
    IClips::iterator it = find(mClips.begin(),mClips.end(),clip);
    ASSERT(it != mClips.end());
    if (it == mClips.begin())
    {
        return IClipPtr();
    }
    --it;
    return *it; 
}

pts Track::getLeftEmptyArea(IClipPtr clip)
{
    ASSERT(UtilList<IClipPtr>(mClips).hasElement(clip));
    pts leftmost = clip->getLeftPts();
    IClipPtr previous = getPreviousClip(clip);
    while (previous && previous->isA<EmptyClip>())
    {
        leftmost = previous->getLeftPts();
        previous = getPreviousClip(previous);
    }
    return leftmost - clip->getLeftPts();
}

pts Track::getRightEmptyArea(IClipPtr clip)
{
    ASSERT(UtilList<IClipPtr>(mClips).hasElement(clip));
    pts rightmost = clip->getRightPts();
    IClipPtr next = getNextClip(clip);
    while (next && next->isA<EmptyClip>())
    {
        rightmost = next->getRightPts();
        next = getNextClip(next);
    }
    return rightmost - clip->getRightPts();
}

//////////////////////////////////////////////////////////////////////////
// STATIC HELPER METHOD
//////////////////////////////////////////////////////////////////////////

//static 
pts Track::getCombinedLength(IClips clips)
{
    int length = 0;
    BOOST_FOREACH( IClipPtr clip, clips )
    {
        if (!clip->isA<Transition>())
        {
            length += clip->getLength();
        }
    }
    return length;
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

bool Track::iterate_atEnd() 
{
    return (mItClips == mClips.end());
}

IClipPtr Track::iterate_get()
{
    ASSERT(mItCurrent);
    return mItCurrent;
}

void Track::iterate_next()
{
    ASSERT(!iterate_atEnd());
    mItClips++;
    if (!iterate_atEnd())
    {
        IClipPtr clip = make_cloned<IClip>(*mItClips);
        mItCurrent = clip;
    }
    else
    {
        mItCurrent.reset();
    }
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void Track::updateClips()
{
	pts position = 0;
    int index = 0;
    // NOTE: any information updated here must also be serialized in the clip,
    //       since this method is not called during (de)serialization, since
    //       the shared_from_this() handling causes problems then.
	BOOST_FOREACH( IClipPtr clip, mClips )
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
void Track::serialize(Archive & ar, const unsigned int version)
{
    ar & boost::serialization::base_object<IControl>(*this);
    ar & mIndex;
    ar & mClips;
}
template void Track::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void Track::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);

} //namespace

