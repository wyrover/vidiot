#include "Clip.h"

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/foreach.hpp>
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
    VAR_DEBUG(*this);
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
    VAR_DEBUG(*this);
}

Clip::Clip(const Clip& other)
    :   wxEvtHandler()
    ,	IControl()
    ,   mRender(make_cloned<model::IControl>(other.mRender))
    ,   mOffset(other.mOffset)
    ,   mLength(other.mLength)
    ,   mTrack(model::TrackPtr())   // Clone is not automatically part of same track!!!
    ,   mLeftPtsInTrack(0)          // Clone is not automatically part of same track!!!
    ,   mLink(other.mLink)
    ,   mLastSetPosition(boost::none)
    ,   mSelected(other.mSelected)
{
    VAR_DEBUG(*this)(other);
}

Clip* Clip::clone()
{ 
    return new Clip(static_cast<const Clip&>(*this)); 
}

Clip::~Clip()
{
    VAR_DEBUG(*this);
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

void Clip::adjustBegin(pts adjustment)
{
    mOffset += adjustment;
    mLength -= adjustment;
    ASSERT(mLength <=  mRender->getNumberOfFrames() - mOffset)(mLength);
    VAR_DEBUG(*this)(adjustment);
}

void Clip::adjustEnd(pts length)
{
    VAR_INFO(this)(length);
    mLength = length;
    ASSERT(mLength <=  mRender->getNumberOfFrames() - mOffset)(mLength);
    VAR_DEBUG(*this)(length);
}

bool Clip::getSelected() const
{
    return mSelected;
}

void Clip::setSelected(bool selected)
{
    mSelected = selected;
    ProcessEvent(EventSelectClip(selected));
}

//////////////////////////////////////////////////////////////////////////
// STATIC HELPER METHOD
//////////////////////////////////////////////////////////////////////////

//static 
pts Clip::getCombinedLength(model::Clips clips)
{
    int length = 0;
    BOOST_FOREACH( model::ClipPtr clip, clips )
    {
        length += clip->getNumberOfFrames();
    }
    return length;
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
    os << &obj << '|' << obj.mOffset << '|' << obj.mLength << '|' << obj.mLeftPtsInTrack << '|' << obj.mSelected;
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
