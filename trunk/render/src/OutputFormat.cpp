// Copyright 2013 Eric Raijmakers.
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
    ,   mDefaultAudioCodec(AudioCodecs::getDefault()->getId())
    ,   mDefaultVideoCodec(VideoCodecs::getDefault()->getId())
    ,   mAudioCodec(AudioCodecs::getDefault())
    ,   mVideoCodec(VideoCodecs::getDefault())
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
    try
    {
        ar & BOOST_SERIALIZATION_NVP(mName);
        ar & BOOST_SERIALIZATION_NVP(mLongName);
        ar & BOOST_SERIALIZATION_NVP(mExtensions);
        ar & BOOST_SERIALIZATION_NVP(mDefaultAudioCodec);
        ar & BOOST_SERIALIZATION_NVP(mDefaultVideoCodec);
        ar & BOOST_SERIALIZATION_NVP(mAudioCodec);
        ar & BOOST_SERIALIZATION_NVP(mVideoCodec);
    }
    catch (boost::archive::archive_exception& e) { VAR_ERROR(e.what());                         throw; }
    catch (boost::exception &e)                  { VAR_ERROR(boost::diagnostic_information(e)); throw; }
    catch (std::exception& e)                    { VAR_ERROR(e.what());                         throw; }
    catch (...)                                  { LOG_ERROR;                                   throw; }
}
template void OutputFormat::serialize<boost::archive::xml_oarchive>(boost::archive::xml_oarchive& ar, const unsigned int archiveVersion);
template void OutputFormat::serialize<boost::archive::xml_iarchive>(boost::archive::xml_iarchive& ar, const unsigned int archiveVersion);

}} //namespace

BOOST_CLASS_EXPORT_IMPLEMENT(model::render::OutputFormat)