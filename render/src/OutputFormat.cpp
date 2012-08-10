#include "OutputFormat.h"

#include "AudioCodec.h"
#include "AudioCodecs.h"
#include "UtilLog.h"
#include "UtilLogStl.h"
#include "UtilSerializeWxwidgets.h"
#include "VideoCodec.h"
#include "VideoCodecs.h"

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
    ,   mAudioCodec(AudioCodecs::find(CODEC_ID_NONE))
    ,   mVideoCodec(VideoCodecs::find(CODEC_ID_NONE))
{
}

OutputFormat::OutputFormat(wxString name, wxString longname, std::list<wxString> extensions, CodecID defaultaudiocodec, CodecID defaultvideocodec)
    :   mName(name)
    ,   mLongName(longname)
    ,   mExtensions(extensions)
    ,   mDefaultAudioCodec(defaultaudiocodec)
    ,   mDefaultVideoCodec(defaultvideocodec)
    ,   mAudioCodec(AudioCodecs::find(defaultaudiocodec))
    ,   mVideoCodec(VideoCodecs::find(defaultvideocodec))
{
}

OutputFormat::OutputFormat(const OutputFormat& other)
    :   mName(other.mName)
    ,   mLongName(other.mLongName)
    ,   mExtensions(other.mExtensions)
    ,   mDefaultAudioCodec(other.mDefaultAudioCodec)
    ,   mDefaultVideoCodec(other.mDefaultVideoCodec)
    ,   mAudioCodec(make_cloned<AudioCodec>(other.getAudioCodec()))
    ,   mVideoCodec(make_cloned<VideoCodec>(other.getVideoCodec()))
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
        (mDefaultVideoCodec == other.mDefaultVideoCodec) &&
        (*mAudioCodec == *other.mAudioCodec) &&
        (*mVideoCodec == *other.mVideoCodec);
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

bool OutputFormat::storeAudio() const
{
    return mAudioCodec && mAudioCodec->getId() != CODEC_ID_NONE;
}

bool OutputFormat::storeVideo() const
{
    return mVideoCodec && mVideoCodec->getId() != CODEC_ID_NONE;
}

VideoCodecPtr OutputFormat::getVideoCodec() const
{
    return mVideoCodec;
}

void OutputFormat::setVideoCodec(VideoCodecPtr codec)
{
    mVideoCodec = codec;
}

AudioCodecPtr OutputFormat::getAudioCodec() const
{
    return mAudioCodec;
}

void OutputFormat::setAudioCodec(AudioCodecPtr codec)
{
    mAudioCodec = codec;
}

AVFormatContext* OutputFormat::getContext() const
{
    AVOutputFormat* format = av_guess_format(mName.c_str(), 0, 0);
    ASSERT(format);
    ASSERT(mAudioCodec->getId() != CODEC_ID_NONE || mVideoCodec->getId() != CODEC_ID_NONE);
    format->audio_codec = mAudioCodec->getId();
    format->video_codec = mVideoCodec->getId();
    AVFormatContext* context = avformat_alloc_context();
    context->oformat = format;
    return context;
}

int OutputFormat::checkCodec(CodecID id) const
{
    if (id == CODEC_ID_NONE) { return 1; }
    AVOutputFormat* format = av_guess_format(mName.c_str(), 0, 0);
    ASSERT(format);
    int supported = avformat_query_codec(format,id,FF_COMPLIANCE_NORMAL);
    VAR_DEBUG(mName)(id)(supported);
    return supported;
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<( std::ostream& os, const OutputFormat& obj )
{
    os  << "OutputFormat:"
        << &obj                   << '|'
        << obj.mName              << '|'
        << obj.mLongName          << '|'
        << obj.mExtensions        << '|'
        << obj.mDefaultAudioCodec << '|'
        << obj.mDefaultVideoCodec << '|'
        << *obj.mAudioCodec       << '|'
        << *obj.mVideoCodec;
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
    ar & mAudioCodec;
    ar & mVideoCodec;
}
template void OutputFormat::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void OutputFormat::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);

}} //namespace