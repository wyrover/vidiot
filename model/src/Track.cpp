// Copyright 2013 Eric Raijmakers.
//
// This file is part of Vidiot.
//
// Vidiot is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Vidiot is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Vidiot. If not, see <http://www.gnu.org/licenses/>.

#include "Track.h"

#include "Calculate.h"
#include "Clip.h"
#include "Constants.h"
#include "EmptyClip.h"
#include "ModelEvent.h"
#include "Node.h"
#include "ProjectModification.h"
#include "TrackEvent.h"
#include "UtilList.h"
#include "UtilLog.h"
#include "UtilLogStl.h"
#include "UtilSet.h"

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

    if (!iterate_atEnd())
    {
        iterate_get()->clean(); // Reset any running threads (particularly, bufferPacketsThread)
    }

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

bool Track::isEmpty() const
{
    BOOST_FOREACH( IClipPtr mClip, mClips )
    {
        if (!mClip->isA<EmptyClip>())
        {
            return false;
        }
    }
    return true;
}

void Track::addClips(IClips clips, IClipPtr position)
{
    VAR_DEBUG(*this)(position)(clips);

    UtilList<IClipPtr>(mClips).addElements(clips,position);

    updateClips();

    // ProcessEvent is used. Model events must be processed synchronously to avoid inconsistent states in
    // the receivers of these events (typically, the view classes in the timeline).
    // Example:
    // 1. Add clip
    // 2. Remove clip again
    // 3. Event of addition is received a bit later. Here the added clip is no longer part of the track. ERROR.
    ProcessEvent(EventAddClips(MoveParameter(self(), position, clips, TrackPtr(), IClipPtr(), IClips()))); // Must be handled immediately

    // This may NOT be called before the add/remove event is sent: updateLength() may cause view updates,
    // which cause accesses to the model. By that time, all views must know the proper list of tracks.
    updateLength();
}

void Track::removeClips(IClips clips)
{
    VAR_DEBUG(*this)(clips);

    BOOST_FOREACH( IClipPtr clip, clips )
    {
        clip->clean();
        boost::dynamic_pointer_cast<Clip>(clip)->setTrackInfo(); // reset
    }

    IClipPtr position = UtilList<IClipPtr>(mClips).removeElements(clips);

    updateClips();

    // ProcessEvent is used. Model events must be processed synchronously to avoid inconsistent states in
    // the receivers of these events (typically, the view classes in the timeline).
    // Example:
    // 1. Add clip
    // 2. Remove clip again
    // 3. Event of addition is received a bit later. Here the added clip is no longer part of the track. ERROR.
    ProcessEvent(EventRemoveClips(MoveParameter(TrackPtr(), IClipPtr(), IClips(), self(), position, clips))); // Must be handled immediately

    // This may NOT be called before the add/remove event is sent: updateLength() may cause view updates,
    // which cause accesses to the model. By that time, all views must know the proper list of tracks.
    updateLength();
}

const IClips& Track::getClips()
{
    return mClips;
}

IClipPtr Track::getClip(pts position) const
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
    if (mHeight != height)
    {
        mHeight = height;
        model::ProjectModification::trigger();
        ProcessEvent(model::EventHeightChanged(height));
    }
}

int Track::getIndex() const
{
    return mIndex;
}

void Track::setIndex(int index)
{
    mIndex = index;
}

std::set<pts> Track::getCuts(const std::set<IClipPtr>& exclude)
{
    std::set<pts> result;
    BOOST_FOREACH( IClipPtr clip, getClips() )
    {
        if (exclude.find(clip) == exclude.end())
        {
            UtilSet<pts>(result).addElements(clip->getCuts(exclude));
        }
    }
    return result;
}

bool Track::isEmptyAt( pts position ) const
{
    return getClip(position)->isA<model::EmptyClip>();
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
        iterate_get()->clean(); // Reset any running threads (particularly, bufferPacketsThread)

        // Step over clips with length 0. These are only used as part of a transition.
        mItClips++;
    }
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void Track::updateClips()
{
    // NOTE: any information updated here must also be serialized in the clip,
    //       since this method is not called during (de)serialization, since
    //       the shared_from_this() handling causes problems then.
    pts position = 0;
    int index = 0;
    IClipPtr prev; // First clip has no previous clip
    IClips::iterator it = mClips.begin();

    while (it != mClips.end())
    {
        IClipPtr clip = *it; // Extract current clip
        IClipPtr next = (++it != mClips.end()) ? *it : IClipPtr(); // Increment iterator first

        boost::dynamic_pointer_cast<Clip>(clip)->setTrackInfo(self(), prev, next, position, index);
        position += clip->getLength();
        index++;
        prev = clip;
    }

    mItClips = mClips.end(); // Must be reset, since it has become invalid
}

void Track::updateLength()
{
    pts length = calculate::combinedLength(mClips);
    if (mCache.length != length)
    {
        ProcessEvent(EventLengthChanged(length)); // Handled immediately
        mCache.length = length;
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

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void Track::serialize(Archive & ar, const unsigned int version)
{
    try
    {
        ar & mIndex;
        ar & mClips;
        ar & mHeight;
    }
    catch (boost::archive::archive_exception& e) { VAR_ERROR(e.what());                         throw; }
    catch (boost::exception &e)                  { VAR_ERROR(boost::diagnostic_information(e)); throw; }
    catch (std::exception& e)                    { VAR_ERROR(e.what());                         throw; }
    catch (...)                                  { LOG_ERROR;                                   throw; }
}
template void Track::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void Track::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);
} //namespace