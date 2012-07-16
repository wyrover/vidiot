#include "VideoCodec.h"

#include <boost/make_shared.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include "UtilLog.h"
#include "UtilLogStl.h"
#include "VideoCodecParameter.h"

namespace model { namespace render {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

VideoCodec::VideoCodec()
    :   mId(CODEC_ID_NONE)
    ,   mParameters()
{
}

VideoCodec::VideoCodec(CodecID id)
    :   mId(id)
    ,   mParameters()
{
}

VideoCodec::VideoCodec(const VideoCodec& other)
    :   mId(other.mId)
    ,   mParameters(make_cloned<ICodecParameter>(other.mParameters))
{
}

VideoCodec::~VideoCodec()
{
}

VideoCodec* VideoCodec::clone() const
{
    return new VideoCodec(static_cast<const VideoCodec&>(*this));
}

//////////////////////////////////////////////////////////////////////////
// OPERATORS
//////////////////////////////////////////////////////////////////////////

bool VideoCodec::operator== (const VideoCodec& other) const
{
    return
        (mId == other.mId) &&
        (mParameters == other.mParameters);
}

//////////////////////////////////////////////////////////////////////////
// PARAMETERS
//////////////////////////////////////////////////////////////////////////

CodecID VideoCodec::getId() const
{
    return mId;
}

VideoCodec& VideoCodec::addParameter(ICodecParameter& parameter)
{
    ICodecParameter* clone = static_cast<ICodecParameter*>(parameter.clone());
    ASSERT(clone);
    ICodecParameterPtr newParam = boost::shared_ptr<ICodecParameter>(clone);
    ASSERT(newParam);
    mParameters.push_back(newParam);

//    mParameters.push_back(make_cloned_ptr<ICodecParameter>(parameter));
    return *this;
}

std::list<ICodecParameterPtr> VideoCodec::getParameters()
{
    return mParameters;
}

void VideoCodec::setParameters( AVCodecContext* codec ) const
{
    BOOST_FOREACH( ICodecParameterPtr parameter, mParameters )
    {
        parameter->set(codec);
    }
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

//template <class T> // todo why is this needed?
//std::ostream& operator<< (std::ostream& os, const std::list<T> obj)
//{
//    os << "{";
//    BOOST_FOREACH( T child, obj )
//    {
//        os << child << " ";
//    }
//    os << "}";
//    return os;
//}

std::ostream& operator<<( std::ostream& os, const VideoCodec& obj )
{
    os  << &obj    << '|'
        << obj.mId << '|'
        << obj.mParameters;
    return os;
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void VideoCodec::serialize(Archive & ar, const unsigned int version)
{
    ar & mId;
    ar & mParameters;
}
template void VideoCodec::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void VideoCodec::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);

}} //namespace