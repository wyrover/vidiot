// Copyright 2013-2016 Eric Raijmakers.
//
// This file is part of Vidiot.
//
// Vidiot is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Vidiot is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Vidiot. If not, see <http://www.gnu.org/licenses/>.

#include "OutputFormat.h"

#include "AudioCodec.h"
#include "AudioCodecs.h"
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
    ,   mFormat(new AVOutputFormat())
    ,   mDefaultAudioCodec(AudioCodecs::getDefault()->getId())
    ,   mDefaultVideoCodec(VideoCodecs::getDefault()->getId())
    ,   mVideoCodec(VideoCodecs::getDefault())
    ,   mAudioCodec(AudioCodecs::getDefault())
{
    ASSERT(mFormat); // mFormat initialized by boost serialization
    memset(mFormat, 0, sizeof(AVOutputFormat));
}

OutputFormat::OutputFormat(const wxString& name, const wxString& longname, const wxStrings& extensions, const AVCodecID& defaultaudiocodec, const AVCodecID& defaultvideocodec)
    :   mName(name)
    ,   mLongName(longname)
    ,   mExtensions(extensions)
    ,   mFormat(new AVOutputFormat())
    ,   mDefaultAudioCodec(defaultaudiocodec)
    ,   mDefaultVideoCodec(defaultvideocodec)
    ,   mVideoCodec(VideoCodecs::find(defaultvideocodec))
    ,   mAudioCodec(AudioCodecs::find(defaultaudiocodec))
{
    VAR_INFO(name)(longname)(extensions)(defaultaudiocodec)(defaultvideocodec);
    ASSERT(mFormat);
    AVOutputFormat* format = av_guess_format(mName.c_str(), 0, 0);
    memcpy(mFormat, format, sizeof(AVOutputFormat));
    mFormat->next = 0;
}

OutputFormat::OutputFormat(const OutputFormat& other)
    :   mName(other.mName)
    ,   mLongName(other.mLongName)
    ,   mExtensions(other.mExtensions)
    ,   mFormat(new AVOutputFormat())
    ,   mDefaultAudioCodec(other.mDefaultAudioCodec)
    ,   mDefaultVideoCodec(other.mDefaultVideoCodec)
    ,   mVideoCodec(make_cloned<VideoCodec>(other.getVideoCodec()))
    ,   mAudioCodec(make_cloned<AudioCodec>(other.getAudioCodec()))
{
    ASSERT(mFormat);
    memcpy(mFormat, other.mFormat, sizeof(AVOutputFormat));
    mFormat->next = 0;
}

OutputFormat* OutputFormat::clone() const
{
    return new OutputFormat(static_cast<const OutputFormat&>(*this));
}

void OutputFormat::onCloned()
{
}

OutputFormat::~OutputFormat()
{
    delete mFormat;
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

OutputFormat& OutputFormat::setName(const wxString& name)
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

wxStrings OutputFormat::getExtensions() const
{
    return mExtensions;
}

AVCodecID OutputFormat::getDefaultAudioCodec() const
{
    return mDefaultAudioCodec;
}

AVCodecID OutputFormat::getDefaultVideoCodec() const
{
    return mDefaultVideoCodec;
}

bool OutputFormat::storeAudio() const
{
    return mAudioCodec && mAudioCodec->getId() != AV_CODEC_ID_NONE;
}

bool OutputFormat::storeVideo() const
{
    return mVideoCodec && mVideoCodec->getId() != AV_CODEC_ID_NONE;
}

VideoCodecPtr OutputFormat::getVideoCodec() const
{
    return mVideoCodec;
}

void OutputFormat::setVideoCodec(const VideoCodecPtr& codec)
{
    mVideoCodec = codec;
}

AudioCodecPtr OutputFormat::getAudioCodec() const
{
    return mAudioCodec;
}

void OutputFormat::setAudioCodec(const AudioCodecPtr& codec)
{
    mAudioCodec = codec;
}

AVFormatContext* OutputFormat::getContext() const
{
    ASSERT(mFormat);
    ASSERT(mAudioCodec->getId() != AV_CODEC_ID_NONE || mVideoCodec->getId() != AV_CODEC_ID_NONE);
    mFormat->audio_codec = mAudioCodec->getId();
    mFormat->video_codec = mVideoCodec->getId();
    AVFormatContext* context = avformat_alloc_context();
    context->oformat = mFormat;
    return context;
}

int OutputFormat::checkCodec(const AVCodecID& id) const
{
    if (id == AV_CODEC_ID_NONE) { return 1; }
    ASSERT(mFormat);
    int supported = avformat_query_codec(mFormat,id,FF_COMPLIANCE_NORMAL);
    VAR_DEBUG(mName)(id)(supported);
    return supported;
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, const OutputFormat& obj)
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
    try
    {
        ar & BOOST_SERIALIZATION_NVP(mName);
        ar & BOOST_SERIALIZATION_NVP(mLongName);
        ar & BOOST_SERIALIZATION_NVP(mExtensions);
        ar & BOOST_SERIALIZATION_NVP(mDefaultAudioCodec);
        ar & BOOST_SERIALIZATION_NVP(mDefaultVideoCodec);
        ar & BOOST_SERIALIZATION_NVP(mAudioCodec);
        ar & BOOST_SERIALIZATION_NVP(mVideoCodec);

        if (Archive::is_loading::value)
        {
            ASSERT(mFormat);
            AVOutputFormat* format = av_guess_format(mName.c_str(), 0, 0);
            memcpy(mFormat, format, sizeof(AVOutputFormat));
            mFormat->next = 0;
        }
    }
    catch (boost::exception &e)                  { VAR_ERROR(boost::diagnostic_information(e)); throw; }
    catch (std::exception& e)                    { VAR_ERROR(e.what());                         throw; }
    catch (...)                                  { LOG_ERROR;                                   throw; }
}
template void OutputFormat::serialize<boost::archive::xml_oarchive>(boost::archive::xml_oarchive& ar, const unsigned int archiveVersion);
template void OutputFormat::serialize<boost::archive::xml_iarchive>(boost::archive::xml_iarchive& ar, const unsigned int archiveVersion);

}} //namespace

BOOST_CLASS_EXPORT_IMPLEMENT(model::render::OutputFormat)
