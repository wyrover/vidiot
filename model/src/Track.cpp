#include "Track.h"

#include "Calculate.h"
#include "Node.h"
#include "Clip.h"
#include "Constants.h"
#include "EmptyClip.h"
#include "TrackEvent.h"
#include "Transition.h"
#include "UtilList.h"
#include "UtilLog.h"
#include "UtilLogStl.h"

namespace model {

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
,   mClips(make_cloned<IClip>(other.mClips))
,   mItClips(mClips.end())
,   mHeight(other.mHeight)
,   mIndex(0)
{
    VAR_DEBUG(this);
    // NOTE: updateClips() is never called here!
}

Track::~Track()
{
    VAR_DEBUG(this);
}

//////////////////////////////////////////////////////////////////////////
// ICONTROL
//////////////////////////////////////////////////////////////////////////

pts Track::getLength() const
{
    return calculate::combinedLength(mClips);
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

    while (lastFrame <= position)
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

    ASSERT_LESS_THAN_EQUALS(position,lastFrame);
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
    BOOST_FOREACH(IClipPtr clip, mClips)
    {
        clip->clean();
    }
    mItClips = mClips.end();
}

//////////////////////////////////////////////////////////////////////////
// HANDLING CLIPS
//////////////////////////////////////////////////////////////////////////

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
        clip->setPrev(IClipPtr());
        clip->setNext(IClipPtr());
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
            LOG_DEBUG << "Found";
            return clip;
        }
        left += length;
    }
    return IClipPtr();
}

//IClips Track::getClips(pts start, pts end)
//{
//    IClips result;
//    pts left = 0;
//    pts right = left;
//    BOOST_FOREACH( IClipPtr clip, mClips )
//    {
//        pts length = clip->getLength();
//        right += length;
//        if ((left <= start && right > start) || // First clip contains pts 'start'
//            (!result.empty()))                  // Start pts already found, keep adding until the break below.
//        {
//            result.push_back(clip);
//        }
//        left += length;
//        if (left >= end) // Subsequent clip is after end
//        {
//            break;
//        }
//    }
//    return result;
//}

IClipPtr Track::getClipByIndex(int index)
{
    IClips::const_iterator it = mClips.begin();
    while (index > 0)
    {
        ++it;
        --index;
    }
    ASSERT(it != mClips.end());
    return *it;
}

pts Track::getLeftEmptyArea(IClipPtr clip)
{
    ASSERT_CONTAINS(mClips,clip);
    pts leftmost = clip->getLeftPts();
    IClipPtr previous = clip->getPrev();
    if (!previous)
    {
        // This clip is the first clip in the track
        return 0;
    }
    while (previous && previous->isA<EmptyClip>())
    {
        leftmost = previous->getLeftPts();
        previous = previous->getPrev();
    }
    return leftmost - clip->getLeftPts();
}

pts Track::getRightEmptyArea(IClipPtr clip)
{
    ASSERT_CONTAINS(mClips,clip);
    pts rightmost = clip->getRightPts();
    IClipPtr next = clip->getNext();
    if (!next)
    {
        // This clip is the last clip in the track
        return (std::numeric_limits<pts>::max)();
    }
    while (next && next->isA<EmptyClip>())
    {
        rightmost = next->getRightPts();
        next = next->getNext();
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

bool Track::iterate_atEnd()
{
    return (mItClips == mClips.end());
}

IClipPtr Track::iterate_get()
{
    ASSERT(*mItClips);
    return *mItClips;
}

void Track::iterate_next()
{
    ASSERT(!iterate_atEnd());
    mItClips++;
    while (!iterate_atEnd() && iterate_get()->getLength() == 0)
    {
        // Step over clips with length 0. These are only used as part of a transition.
        mItClips++;
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
    IClipPtr next;
    IClipPtr prev;
    BOOST_FOREACH( IClipPtr clip, mClips )
    {
        if (prev)
        {
            prev->setNext(clip);
        }
        clip->setTrack(shared_from_this(), position, index);
        clip->setPrev(prev);
        clip->setNext(IClipPtr()); // Will be overwritten for next clip, if any
        position += clip->getLength();
        index++;
        prev = clip;
    }
    mItClips = mClips.end(); // Must be reset, since it has become invalid
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<( std::ostream& os, const Track& obj )
{
    os << &obj << '|' << obj.mIndex << '|' << obj.mHeight << '|' << obj.mClips;
    return os;
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void Track::serialize(Archive & ar, const unsigned int version)
{
    ar & mIndex;
    ar & mClips;
}
template void Track::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void Track::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);
} //namespace