#include "Transition.h"

#include "ClipEvent.h"
#include "Track.h"

#include "UtilCloneable.h"
#include "UtilLog.h"
#include "UtilSerializeBoost.h"

namespace model {
//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

Transition::Transition()
    :   IClip()
    ,   mFramesLeft(0)
    ,   mFramesRight(0)
    ,   mLastSetPosition(boost::none)
    ,   mGeneratedPts(0)
    ,   mTrack()
    ,   mIndex(0)
    ,   mLeftPtsInTrack(0)
    ,   mSelected(false)
    ,   mDragged(false)
{
    VAR_DEBUG(this);
}

Transition::Transition(pts nFramesLeft, pts nFramesRight)
    :   IClip()
    ,   mFramesLeft(nFramesLeft)
    ,   mFramesRight(nFramesRight)
    ,   mLastSetPosition(boost::none)
    ,   mGeneratedPts(0)
    ,   mTrack()
    ,   mIndex(0)
    ,   mLeftPtsInTrack(0)
    ,   mSelected(false)
    ,   mDragged(false)
{
    VAR_DEBUG(this);
}

Transition::Transition(const Transition& other)
    :   IClip()
    ,   mFramesLeft(other.mFramesLeft)
    ,   mFramesRight(other.mFramesRight)
    ,   mLastSetPosition(boost::none)
    ,   mGeneratedPts(0)
    ,   mTrack()            // Clone is not automatically part of same track!!!
    ,   mIndex(0)           // Clone is not automatically part of same track!!!
    ,   mLeftPtsInTrack(0)  // Clone is not automatically part of same track!!!
    ,   mSelected(other.mSelected)
    ,   mDragged(false)
{
    VAR_DEBUG(*this);
}

Transition* Transition::clone() const
{
    return new Transition(static_cast<const Transition&>(*this));
}

Transition::~Transition()
{
    VAR_DEBUG(this);
}

//////////////////////////////////////////////////////////////////////////
// ICONTROL
//////////////////////////////////////////////////////////////////////////

pts Transition::getLength() const
{
    return mFramesLeft + mFramesRight;
}

void Transition::moveTo(pts position)
{
    mLastSetPosition.reset(position);
}

wxString Transition::getDescription() const
{
    return "";
}

void Transition::clean()
{
}

//////////////////////////////////////////////////////////////////////////
// ICLIP
//////////////////////////////////////////////////////////////////////////

void Transition::setTrack(TrackPtr track, pts trackPosition, unsigned int index)
{
    mIndex = index;
    mTrack = track;
    mLeftPtsInTrack = trackPosition;
}

TrackPtr Transition::getTrack()
{
    return mTrack.lock();
}

pts Transition::getLeftPts() const
{
    return mLeftPtsInTrack;
}

pts Transition::getRightPts() const
{
    return mLeftPtsInTrack + mFramesLeft + mFramesRight;
}

void Transition::setLink(IClipPtr link)
{
    // Transitions may never be linked to anything
    ASSERT(!link)(link);
}

IClipPtr Transition::getLink() const
{
    return IClipPtr(); // Transition never has a link
}

pts Transition::getMinAdjustBegin() const
{
    ASSERT(mTrack.lock()); // Do not call when the transition is not part of a track: the algorithm doesn't work then (for instance, with clones)
    pts result = std::numeric_limits<pts>().min();
    if (getLeft() > 0)
    {
        ASSERT(getPrev()); // Avoid bugs where this method is called before a transition has been made part of a track
        result = -1 *  getPrev()->getLength();
        if (getRight() > 0)
        {
            ASSERT(getNext());
            result = std::max(result, getNext()->getMinAdjustBegin());
        }
    }
    else
    {
        result = 0; // InOnlyTransition: Cannot enlarge to the left
    }
    return result;
}

pts Transition::getMaxAdjustBegin() const
{
    ASSERT(mTrack.lock()); // Do not call when the transition is not part of a track: the algorithm doesn't work then (for instance, with clones)
    return getLeft();
}

void Transition::adjustBegin(pts adjustment)
{
    VAR_DEBUG(*this)(adjustment);
    ASSERT(!getTrack())(getTrack()); // Otherwise, this action needs an event indicating the change to the track(view). Instead, tracks are updated by replacing clips.
    mFramesLeft -= adjustment;
}

pts Transition::getMinAdjustEnd() const
{
    ASSERT(mTrack.lock()); // Do not call when the transition is not part of a track: the algorithm doesn't work then (for instance, with clones)
    return -getRight();
}

pts Transition::getMaxAdjustEnd() const
{
    ASSERT(mTrack.lock()); // Do not call when the transition is not part of a track: the algorithm doesn't work then (for instance, with clones)
    pts result = std::numeric_limits<pts>().max();
    if (getRight() > 0)
    {
        ASSERT(getNext()); // Avoid bugs where this method is called before a transition has been made part of a track
        result = getNext()->getLength();
        if (getLeft() > 0)
        {
            ASSERT(getPrev());
            result = std::min(result, getPrev()->getMaxAdjustEnd());
        }
    }
    else
    {
        result = 0;
        // OutOnlyTransition: Cannot enlarge to the right
    }
    return result;
}

void Transition::adjustEnd(pts adjustment)
{
    VAR_DEBUG(*this)(adjustment);
    ASSERT(!getTrack())(getTrack()); // Otherwise, this action needs an event indicating the change to the track(view). Instead, tracks are updated by replacing clips.
    mFramesRight += adjustment;
}

TransitionPtr Transition::getInTransition() const
{
    return TransitionPtr();
}

TransitionPtr Transition::getOutTransition() const
{
    return TransitionPtr();
}

bool Transition::getSelected() const
{
    return mSelected;
}

void Transition::setSelected(bool selected)
{
    mSelected = selected;
    ProcessEvent(EventSelectClip(selected));
}

bool Transition::getDragged() const
{
    return mDragged;
}

void Transition::setDragged(bool dragged)
{
    mDragged = dragged;
    ProcessEvent(EventDragClip(dragged));
}

pts Transition::getGenerationProgress() const
{
    return mGeneratedPts;
}

void Transition::setGenerationProgress(pts progress)
{
    if (mGeneratedPts != progress)
    {
        mGeneratedPts = progress;
        ProcessEvent(DebugEventRenderProgress(mGeneratedPts));
    }
}

void Transition::invalidateLastSetPosition()
{
    mLastSetPosition = boost::none;
}

boost::optional<pts> Transition::getLastSetPosition() const
{
    return mLastSetPosition;
}

std::set<pts> Transition::getCuts(const std::set<IClipPtr>& exclude) const
{
    std::set<pts> result;
    result.insert(getLeftPts());
    result.insert(getTouchPosition());
    result.insert(getRightPts());
    return result;
}

//////////////////////////////////////////////////////////////////////////
// TRANSITION
//////////////////////////////////////////////////////////////////////////

pts Transition::getTouchPosition() const
{
    return getLeftPts() + getLeft();
}

pts Transition::getLeft() const
{
    return mFramesLeft;
}

pts Transition::getRight() const
{
    return mFramesRight;
}

model::IClipPtr Transition::makeLeftClip() const
{
    model::IClipPtr result;
    if (getLeft() > 0)
    {
        ASSERT(getPrev());
        result = boost::const_pointer_cast<model::IClip>(make_cloned<const model::IClip>(getPrev()));
        result->adjustBegin(result->getLength());
        result->adjustEnd(getLength());
    }
    return result;
}

model::IClipPtr Transition::makeRightClip() const
{
    model::IClipPtr result;
    if (getRight() > 0)
    {
        ASSERT(getNext());
        result = boost::const_pointer_cast<model::IClip>(make_cloned<const model::IClip>(getNext()));
        result->adjustEnd(- result->getLength());
        result->adjustBegin(-getLength());
    }
    return result;
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<( std::ostream& os, const Transition& obj )
{
    // Keep order same as Clip and EmptyClip for 'DumpSequence' method
    os << &obj << '|' << obj.mTrack.lock() << '|'  << std::setw(8) << ' ' << '|' << std::setw(6) << obj.mIndex << '|' << std::setw(6) << obj.mLeftPtsInTrack << '|' << std::setw(6) << obj.mFramesLeft << '|' << std::setw(6) << obj.mFramesRight << '|' << obj.mSelected;
    return os;
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void Transition::serialize(Archive & ar, const unsigned int version)
{
    ar & boost::serialization::base_object<IClip>(*this);
    ar & mFramesLeft;
    ar & mFramesRight;
    ar & mTrack;
    ar & mLeftPtsInTrack;
    ar & mIndex;
    // NOT: mSelected. After loading, nothing is selected.
}
template void Transition::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void Transition::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);
} //namespace