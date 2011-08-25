#include "Transition.h"

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/make_shared.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include "ClipEvent.h"
#include "Track.h"
#include "UtilLog.h"
#include "UtilSerializeBoost.h"

namespace model {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

Transition::Transition()
    :   IClip()
    ,   mLeft()
    ,   mRight()
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

Transition::Transition(IClipPtr left, pts nFramesLeft, IClipPtr right, pts nFramesRight)
    :   IClip()
    ,   mLeft(left)
    ,   mRight(right)
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
    ,   mLeft(other.mLeft)
    ,   mRight(other.mRight)
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

Transition* Transition::clone()
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

pts Transition::getLength()
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
    FATAL("Not allowed for transition");
}

IClipPtr Transition::getLink() const
{
    return IClipPtr(); // Transition never has a link
}

pts Transition::getMinAdjustBegin() const
{
    pts result = 0;
    if (getLeftClip())
    {
        result = -getLeftClip()->getLength();
    }
    return result;
}

pts Transition::getMaxAdjustBegin() const
{
    return getLeft();
}

void Transition::adjustBegin(pts adjustment)
{
    VAR_DEBUG(*this)(adjustment);
    ASSERT_MORE_THAN_EQUALS(adjustment,getMinAdjustBegin());
    mFramesLeft += adjustment;
}

pts Transition::getMinAdjustEnd() const
{
    return -getRight();
}

pts Transition::getMaxAdjustEnd() const
{
    pts result = 0;
    if (getRightClip())
    {
        result = getRightClip()->getLength();
    }
    return 0;
}

void Transition::adjustEnd(pts adjustment)
{
    VAR_DEBUG(*this)(adjustment);
    ASSERT_MORE_THAN_EQUALS(adjustment,getMinAdjustEnd());
    mFramesRight += adjustment;
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

//////////////////////////////////////////////////////////////////////////
// TRANSITION
//////////////////////////////////////////////////////////////////////////

pts Transition::getLeft() const
{
    return mFramesLeft;
}

pts Transition::getRight() const
{
    return mFramesRight;
}

IClipPtr Transition::getLeftClip() const
{
    return mLeft;
}

IClipPtr Transition::getRightClip() const
{
    return mRight;
}
//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<( std::ostream& os, const Transition& obj )
{
    // Keep order same as Clip and EmptyClip for 'DumpSequence' method
    os << &obj << '|' <<obj.mTrack.lock() << '|' << std::setw(3) << obj.mIndex << '|' << std::setw(6) << obj.mLeftPtsInTrack << '|' << obj.mLeft << '|' << std::setw(3) << obj.mFramesLeft << '|' << obj.mRight << '|' << std::setw(3) << obj.mFramesRight << '|' << obj.mSelected;
    return os;
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION 
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void Transition::serialize(Archive & ar, const unsigned int version)
{
    ar & boost::serialization::base_object<IClip>(*this);
    ar & mLeft;
    ar & mRight;
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

