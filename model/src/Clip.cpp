#include "Clip.h"
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include "UtilLog.h"
#include "AProjectViewNode.h"

namespace model {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

Clip::Clip()
    :	IControl()
    ,   mRender()
    ,   mOffset(0)
    ,   mLength(0)
{ 
    VAR_DEBUG(this);
}

Clip::Clip(IControlPtr clip)
    :	IControl()
    ,   mRender(clip)
    ,   mOffset(0)
    ,   mLength(0)
{ 
    VAR_DEBUG(this);
}

Clip::~Clip()
{
}

//////////////////////////////////////////////////////////////////////////
// ICONTROL
//////////////////////////////////////////////////////////////////////////

boost::int64_t Clip::getNumberOfFrames()
{
    return mRender->getNumberOfFrames() - mOffset; 
}

void Clip::moveTo(boost::int64_t position)
{
    VAR_DEBUG(this)(position);
    mRender->moveTo(mOffset + position);
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
}
template void Clip::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void Clip::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);

} //namespace
