#include "AudioCodec.h"

#include <boost/make_shared.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include "UtilLog.h"
#include "UtilLogStl.h"
#include "AudioCodecParameter.h"

namespace model { namespace render {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

AudioCodec::AudioCodec()
    :   mId(CODEC_ID_NONE)
    ,   mParameters()
{
}

AudioCodec::AudioCodec(CodecID id)
    :   mId(id)
    ,   mParameters()
{
}

AudioCodec::AudioCodec(const AudioCodec& other)
    :   mId(other.mId)
    ,   mParameters(make_cloned<ICodecParameter>(other.mParameters))
{
}

AudioCodec::~AudioCodec()
{
}

AudioCodec* AudioCodec::clone() const
{
    return new AudioCodec(static_cast<const AudioCodec&>(*this));
}

//////////////////////////////////////////////////////////////////////////
// PARAMETERS
//////////////////////////////////////////////////////////////////////////

CodecID AudioCodec::getId() const
{
    return mId;
}

AudioCodec& AudioCodec::addParameter(ICodecParameter& parameter)
{
    mParameters.push_back(make_cloned_ptr<ICodecParameter>(parameter));
    return *this;
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

std::ostream& operator<<( std::ostream& os, const AudioCodec& obj )
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
void AudioCodec::serialize(Archive & ar, const unsigned int version)
{
    ar & mId;
    ar & mParameters;
}
template void AudioCodec::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void AudioCodec::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);

}} //namespace