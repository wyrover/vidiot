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
    :   Clip()
    ,   mFramesLeft(-1)
    ,   mFramesRight(-1)
{
    VAR_DEBUG(*this);
}

void Transition::init(pts nFramesLeft, pts nFramesRight)
{
    mFramesLeft = nFramesLeft;
    mFramesRight = nFramesRight;
    VAR_DEBUG(this)(nFramesLeft)(nFramesRight);
}

Transition::Transition(const Transition& other)
    :   Clip(other)
    ,   mFramesLeft(other.mFramesLeft)
    ,   mFramesRight(other.mFramesRight)
{
    VAR_DEBUG(*this)(other);
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
    setNewStartPosition(position);
}

//////////////////////////////////////////////////////////////////////////
// ICLIP
//////////////////////////////////////////////////////////////////////////

void Transition::setLink(IClipPtr link)
{
     ASSERT(!link)(link); // Transitions may never be linked to anything
}

pts Transition::getMinAdjustBegin() const
{
    ASSERT(hasTrack()); // Do not call when the transition is not part of a track: the algorithm doesn't work then (for instance, with clones)
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
    ASSERT(hasTrack()); // Do not call when the transition is not part of a track: the algorithm doesn't work then (for instance, with clones)
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
    ASSERT(hasTrack()); // Do not call when the transition is not part of a track: the algorithm doesn't work then (for instance, with clones)
    return -getRight();
}

pts Transition::getMaxAdjustEnd() const
{
    ASSERT(hasTrack()); // Do not call when the transition is not part of a track: the algorithm doesn't work then (for instance, with clones)
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

std::set<pts> Transition::getCuts(const std::set<IClipPtr>& exclude) const
{
    std::set<pts> result;
    result.insert(getLeftPts());
    result.insert(getTouchPosition());
    result.insert(getRightPts());
    return result;
}

std::ostream& Transition::dump(std::ostream& os) const
{
    os << *this;
    return os;
}

char* Transition::getType() const
{
    return "Trans";
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
    // Keep order same as Clip for 'dump' method
    os << static_cast<const Clip&>(obj) << '|'
       << std::setw(6) << obj.mFramesLeft << '|'
       << std::setw(6) << obj.mFramesRight;
    return os;
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void Transition::serialize(Archive & ar, const unsigned int version)
{
    ar & boost::serialization::base_object<Clip>(*this);
    ar & mFramesLeft;
    ar & mFramesRight;
    // NOT: mSelected. After loading, nothing is selected.
}
template void Transition::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void Transition::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);
} //namespace