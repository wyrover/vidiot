#include "OutputFormat.h"

#include <boost/make_shared.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include "UtilSerializeWxwidgets.h"
#include "AudioCodec.h"
#include "VideoCodec.h"
#include "UtilLog.h"

namespace model { namespace render {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

OutputFormat::OutputFormat()
    :   mName()
    ,   mLongName()
    ,   mExtension()
    ,   mAudioCodec()
    ,   mVideoCodec()
{
}

OutputFormat::OutputFormat(wxString name, wxString longname, wxString extension, AudioCodecPtr audiocodec, VideoCodecPtr videocodec)
    :   mName(name)
    ,   mLongName(longname)
    ,   mExtension(extension)
    ,   mAudioCodec(audiocodec)
    ,   mVideoCodec(videocodec)
{
}

OutputFormat::OutputFormat(const OutputFormat& other)
    :   mName(other.mName)
    ,   mLongName(other.mLongName)
    ,   mExtension(other.mExtension)
    ,   mAudioCodec(other.mAudioCodec ? make_cloned<AudioCodec>(other.mAudioCodec) : AudioCodecPtr())
    ,   mVideoCodec(other.mVideoCodec ? make_cloned<VideoCodec>(other.mVideoCodec) : VideoCodecPtr())
{
}

OutputFormat::~OutputFormat()
{
}

//OutputFormat* OutputFormat::clone() const
//{
//    return new OutputFormat(static_cast<const OutputFormat&>(*this));
//}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

OutputFormat& OutputFormat::setName(wxString name)
{
    mName = name;
    return *this;
}

wxString OutputFormat::getName() const
{
    return mName;
}

wxString OutputFormat::getLongName() const
{
    return mLongName;
}

wxString OutputFormat::getExtension() const
{
    return mExtension;
}

AudioCodecPtr OutputFormat::getAudioCodec() const
{
    return mAudioCodec;
}

VideoCodecPtr OutputFormat::getVideoCodec() const
{
    return mVideoCodec;
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<( std::ostream& os, const OutputFormat& obj )
{
    os  << &obj            << '|'
        << obj.mName       << '|'
        << obj.mLongName   << '|'
        << obj.mExtension  << '|'
        << obj.mAudioCodec << '|'
        << obj.mVideoCodec;
    return os;
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void OutputFormat::serialize(Archive & ar, const unsigned int version)
{
    ar & mName;
    ar & mLongName;
    ar & mExtension;
    ar & mAudioCodec;
    ar & mVideoCodec;
}
template void OutputFormat::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void OutputFormat::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);

}} //namespace