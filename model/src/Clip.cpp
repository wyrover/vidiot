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
    ,   mLeftPtsInTrack(0)
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
    ,   mLeftPtsInTrack(0)
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
    ,   mLeftPtsInTrack(other.mLeftPtsInTrack)
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

pts Clip::getNumberOfFrames()
{
    return mLength; 
}

void Clip::moveTo(pts position)
{
    VAR_DEBUG(this)(position);
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

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<( std::ostream& os, const Clip& obj )
{
    os << '[' << &obj << ',' << obj.mOffset << ',' << obj.mLength << ',' << obj.mLeftPtsInTrack << ']';
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
}
template void Clip::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void Clip::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);

} //namespace
