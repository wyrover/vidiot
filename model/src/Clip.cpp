#include "Clip.h"

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include "UtilLog.h"
#include "Track.h"

namespace model {

DEFINE_EVENT(EVENT_SELECT_CLIP, EventSelectClip, bool);

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

Clip::Clip()
    :   wxEvtHandler()
    ,	IControl()
    ,   mRender()
    ,   mOffset(0)
    ,   mLength(-1)
    ,   mTrack()
    ,   mLeftPtsInTrack(0)
    ,   mLink()
    ,   mLastSetPosition(boost::none)
    ,   mSelected(false)
{ 
    VAR_DEBUG(this);
}

Clip::Clip(IControlPtr clip)
    :   wxEvtHandler()
    ,	IControl()
    ,   mRender(clip)
    ,   mOffset(0)
    ,   mLength(-1)
    ,   mTrack()
    ,   mLeftPtsInTrack(0)
    ,   mLink()
    ,   mLastSetPosition(boost::none)
    ,   mSelected(false)
{ 
    mLength = mRender->getNumberOfFrames() - mOffset;
    VAR_DEBUG(this)(*this);
}

Clip::Clip(const Clip& other)
    :   wxEvtHandler()
    ,	IControl()
    ,   mRender(make_cloned<model::IControl>(other.mRender))
    ,   mOffset(other.mOffset)
    ,   mLength(other.mLength)
    ,   mTrack(other.mTrack)
    ,   mLeftPtsInTrack(other.mLeftPtsInTrack)
    ,   mLink(other.mLink)
    ,   mLastSetPosition(boost::none)
    ,   mSelected(other.mSelected)
{
    VAR_DEBUG(this)(other);
}

Clip* Clip::clone()
{ 
    return new Clip(static_cast<const Clip&>(*this)); 
}

Clip::~Clip()
{
    VAR_DEBUG(this);
}

//////////////////////////////////////////////////////////////////////////
// ICONTROL
//////////////////////////////////////////////////////////////////////////

pts Clip::getNumberOfFrames()
{
    return mLength; 
}

void Clip::moveTo(pts position)
{
    VAR_DEBUG(this)(position);
    mLastSetPosition.reset(position);
    mRender->moveTo(mOffset + position);
}

//////////////////////////////////////////////////////////////////////////
// TRACK
//////////////////////////////////////////////////////////////////////////

void Clip::setTrack(TrackPtr track, pts trackPosition)
{
    mTrack = track;
    mLeftPtsInTrack = trackPosition;
}

TrackPtr Clip::getTrack()
{
    return mTrack;
}

pts Clip::getLeftPts() const
{
    return mLeftPtsInTrack;
}

pts Clip::getRightPts() const
{
    return mLeftPtsInTrack + mLength;
}

//////////////////////////////////////////////////////////////////////////
// LINK
//////////////////////////////////////////////////////////////////////////

void Clip::setLink(ClipPtr link)
{
    mLink = link;
}

ClipPtr Clip::getLink() const
{
    return mLink;
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

void Clip::adjustBeginPoint(pts adjustment)
{
    VAR_INFO(this)(adjustment);
    mOffset += adjustment;
    mLength -= adjustment;
    ASSERT(mLength <=  mRender->getNumberOfFrames() - mOffset)(mLength);
    VAR_DEBUG(this)(*this);
}

void Clip::adjustEndPoint(pts adjustment)
{
    VAR_INFO(this)(adjustment);
    mLength += adjustment;
    ASSERT(mLength <=  mRender->getNumberOfFrames() - mOffset);
    VAR_DEBUG(this)(*this);
}

bool Clip::getSelected() const
{
    return mSelected;
}

void Clip::setSelected(bool selected)
{
    mSelected = selected;
    QueueEvent(new EventSelectClip(selected));
}

//////////////////////////////////////////////////////////////////////////
// CURRENT POSITION HANDLING
//////////////////////////////////////////////////////////////////////////

void Clip::invalidateLastSetPosition()
{
    mLastSetPosition = boost::none;
}

boost::optional<pts> Clip::getLastSetPosition() const
{
    return mLastSetPosition;
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<( std::ostream& os, const Clip& obj )
{
    os << '[' << &obj << ',' << obj.mOffset << ',' << obj.mLength << ',' << obj.mLeftPtsInTrack << ',' << obj.mSelected << ']';
    return os;
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION 
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void Clip::serialize(Archive & ar, const unsigned int version)
{
    ar & boost::serialization::base_object<IControl>(*this);
    ar & mRender;
    ar & mOffset;
    ar & mLength;
    ar & mTrack;
    ar & mLeftPtsInTrack;
    ar & mLink;
    // NOT: mSelected. After loading, nothing is selected.
}
template void Clip::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void Clip::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);

} //namespace
