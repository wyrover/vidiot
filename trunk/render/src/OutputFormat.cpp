#include "OutputFormat.h"

#include <boost/serialization/list.hpp>
#include "AudioCodec.h"
#include "UtilLog.h"
#include "UtilLogStl.h"
#include "UtilSerializeWxwidgets.h"
#include "VideoCodec.h"

namespace model { namespace render {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

OutputFormat::OutputFormat()
    :   mName()
    ,   mLongName()
    ,   mExtensions()
    ,   mDefaultAudioCodec(CODEC_ID_NONE)
    ,   mDefaultVideoCodec(CODEC_ID_NONE)
{
}

OutputFormat::OutputFormat(wxString name, wxString longname, std::list<wxString> extensions, CodecID defaultaudiocodec, CodecID defaultvideocodec)
    :   mName(name)
    ,   mLongName(longname)
    ,   mExtensions(extensions)
    ,   mDefaultAudioCodec(defaultaudiocodec)
    ,   mDefaultVideoCodec(defaultvideocodec)
{
}

OutputFormat::OutputFormat(const OutputFormat& other)
    :   mName(other.mName)
    ,   mLongName(other.mLongName)
    ,   mExtensions(other.mExtensions)
    ,   mDefaultAudioCodec(other.mDefaultAudioCodec)
    ,   mDefaultVideoCodec(other.mDefaultVideoCodec)
{
}

OutputFormat::~OutputFormat()
{
}

//////////////////////////////////////////////////////////////////////////
// OPERATORS
//////////////////////////////////////////////////////////////////////////

bool OutputFormat::operator== (const OutputFormat& other) const
{
    return
        (mName == other.mName) &&
        (mLongName == other.mLongName) &&
        (mExtensions == other.mExtensions) &&
        (mDefaultAudioCodec == other.mDefaultAudioCodec) &&
        (mDefaultVideoCodec == other.mDefaultVideoCodec);
}

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

std::list<wxString> OutputFormat::getExtensions() const
{
    return mExtensions;
}

CodecID OutputFormat::getDefaultAudioCodec() const
{
    return mDefaultAudioCodec;
}

CodecID OutputFormat::getDefaultVideoCodec() const
{
    return mDefaultVideoCodec;
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<( std::ostream& os, const OutputFormat& obj )
{
    os  << &obj                   << '|'
        << obj.mName              << '|'
        << obj.mLongName          << '|'
        << obj.mExtensions        << '|'
        << obj.mDefaultAudioCodec << '|'
        << obj.mDefaultVideoCodec;
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
    ar & mExtensions;
    ar & mDefaultAudioCodec;
    ar & mDefaultVideoCodec;
}
template void OutputFormat::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void OutputFormat::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);

}} //namespace