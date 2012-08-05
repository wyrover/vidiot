#include "AudioCodec.h"

#include "AudioCodecParameter.h"
#include "UtilList.h"
#include "UtilLog.h"
#include "UtilLogStl.h"

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
// OPERATORS
//////////////////////////////////////////////////////////////////////////

bool AudioCodec::operator== (const AudioCodec& other) const
{
    return (mId == other.mId) && equals(mParameters,other.mParameters);
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

ICodecParameters AudioCodec::getParameters()
{
    return mParameters;
}

void AudioCodec::setParameters( AVCodecContext* codec ) const
{
    BOOST_FOREACH( ICodecParameterPtr parameter, mParameters )
    {
        parameter->set(codec);
    }
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

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