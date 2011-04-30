#include "Transition.h"

#include <boost/make_shared.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include "UtilLog.h"
#include "Track.h"

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
    ,   mSelected(false)
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
    // todo
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
    return mTrack;
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
    FATAL("Not allowed for transition");
    return IClipPtr();
}

pts Transition::getMinAdjustBegin() const
{
    //todo
    NIY;
    return 0;
}

pts Transition::getMaxAdjustBegin() const
{
    NIY;
    return 0;
}

void Transition::adjustBegin(pts adjustment)
{
    NIY;
    VAR_DEBUG(*this)(adjustment);
}

pts Transition::getMinAdjustEnd() const
{
    NIY;
    return 0;
}

pts Transition::getMaxAdjustEnd() const
{
    NIY;
    return 0;
}

void Transition::adjustEnd(pts adjustment)
{
    NIY;
    VAR_DEBUG(*this)(adjustment);
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

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<( std::ostream& os, const Transition& obj )
{
    os << &obj << '|' << obj.mFramesLeft << '|' << obj.mFramesRight << '|' << obj.mTrack << '|' << obj.mLeftPtsInTrack;
    return os;
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION 
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void Transition::serialize(Archive & ar, const unsigned int version)
{
    ar & boost::serialization::base_object<IControl>(*this);
    ar & mFramesLeft;
    ar & mFramesRight;
    ar & mTrack;
    ar & mLeftPtsInTrack;
}
template void Transition::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void Transition::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);

} //namespace

