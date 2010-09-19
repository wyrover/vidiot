#include "Clip.h"

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include "UtilLog.h"

namespace model {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

Clip::Clip()
    :	IControl()
    ,   mRender()
    ,   mOffset(0)
    ,   mLength(-1)
    ,   mLink()
{ 
    VAR_DEBUG(this);
}

Clip::Clip(IControlPtr clip)
    :	IControl()
    ,   mRender(clip)
    ,   mOffset(0)
    ,   mLength(-1)
    ,   mLink()
{ 
    VAR_DEBUG(this);
}

Clip::Clip(const Clip& other)
:   IControl()
,   mRender(make_cloned<model::IControl>(other.mRender))
,   mOffset(other.mOffset)
,   mLength(other.mLength)
,   mLink(other.mLink)
{
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
    if (mLength == -1)
    {
        mLength = mRender->getNumberOfFrames() - mOffset;
    }
    return mLength; 
}

void Clip::moveTo(boost::int64_t position)
{
    VAR_DEBUG(this)(position);
    mRender->moveTo(mOffset + position);
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
    mOffset = offset;
}

void Clip::setLength(boost::int64_t length)
{
    mLength = length;
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<( std::ostream& os, const Clip& obj )
{
    os << '[' << obj.mOffset << ',' << obj.mLength << ']';
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
    ar & mLink;
}
template void Clip::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void Clip::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);

} //namespace
