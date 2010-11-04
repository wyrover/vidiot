#include "Clip.h"

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include "UtilLog.h"
#include "Track.h"

namespace model {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

Clip::Clip()
    :	IControl()
    ,   mRender()
    ,   mOffset(0)
    ,   mLength(-1)
    ,   mTrack()
    ,   mTrackPosition(0)
    ,   mLink()
{ 
    VAR_DEBUG(this);
}

Clip::Clip(IControlPtr clip)
    :	IControl()
    ,   mRender(clip)
    ,   mOffset(0)
    ,   mLength(-1)
    ,   mTrack()
    ,   mTrackPosition(0)
    ,   mLink()
{ 
    mLength = mRender->getNumberOfFrames() - mOffset;
    VAR_DEBUG(this)(*this);
}

Clip::Clip(const Clip& other)
    :   IControl()
    ,   mRender(make_cloned<model::IControl>(other.mRender))
    ,   mOffset(other.mOffset)
    ,   mLength(other.mLength)
    ,   mTrack(other.mTrack)
    ,   mTrackPosition(other.mTrackPosition)
    ,   mLink(other.mLink)
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

boost::int64_t Clip::getNumberOfFrames()
{
    return mLength; 
}

void Clip::moveTo(boost::int64_t position)
{
    VAR_DEBUG(this)(position);
    mRender->moveTo(mOffset + position);
}

//////////////////////////////////////////////////////////////////////////
// TRACK
//////////////////////////////////////////////////////////////////////////

void Clip::setTrack(TrackPtr track, boost::int64_t trackPosition)
{
    mTrack = track;
    mTrackPosition = trackPosition;
}

TrackPtr Clip::getTrack()
{
    return mTrack;
}

boost::int64_t Clip::getLeftPts() const
{
    return mTrackPosition;
}

boost::int64_t Clip::getRightPts() const
{
    return mTrackPosition + mLength;
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

boost::int64_t Clip::getOffset()
{
    return mOffset;
}

void Clip::setOffset(boost::int64_t offset)
{
    VAR_INFO(this)(offset);
    mOffset = offset;
    VAR_DEBUG(this)(*this);
}

void Clip::setLength(boost::int64_t length)
{
    VAR_INFO(this)(length);
    mLength = length;
    ASSERT(mLength>0)(mLength);
    VAR_DEBUG(this)(*this);
}

void Clip::adjustBeginPoint(boost::int64_t adjustment)
{
    VAR_INFO(this)(adjustment);
    mOffset += adjustment;
    mLength -= adjustment;
    ASSERT(mLength <=  mRender->getNumberOfFrames() - mOffset)(mLength);
    VAR_DEBUG(this)(*this);
}

void Clip::adjustEndPoint(boost::int64_t adjustment)
{
    VAR_INFO(this)(adjustment);
    mLength += adjustment;
    ASSERT(mLength <=  mRender->getNumberOfFrames() - mOffset);
    VAR_DEBUG(this)(*this);
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<( std::ostream& os, const Clip& obj )
{
    os << '[' << &obj << ',' << obj.mOffset << ',' << obj.mLength << ',' << obj.mTrackPosition << ']';
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
    ar & mTrackPosition;
    ar & mLink;
}
template void Clip::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void Clip::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);

} //namespace
