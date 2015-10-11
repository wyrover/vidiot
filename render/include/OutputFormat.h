// Copyright 2013-2015 Eric Raijmakers.
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

#pragma once

namespace model { namespace render {

class OutputFormat
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    OutputFormat();

    explicit OutputFormat(const wxString& name, const wxString& longname, const wxStrings& extensions, const AVCodecID& defaultaudiocodec, const AVCodecID& defaultvideocodec);

    OutputFormat(const OutputFormat& other);

    virtual OutputFormat* clone() const;

    virtual void onCloned();

    virtual ~OutputFormat();

    //////////////////////////////////////////////////////////////////////////
    // OPERATORS
    //////////////////////////////////////////////////////////////////////////

    bool operator== (const OutputFormat& other) const;

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    OutputFormat& setName(const wxString& name);
    wxString getName() const;
    wxString getLongName() const;
    wxStrings getExtensions() const;
    AVCodecID getDefaultAudioCodec() const;
    AVCodecID getDefaultVideoCodec() const;

    bool storeAudio() const; ///< \return true if audio output is requested
    bool storeVideo() const; ///< \return true if video output is requested

    VideoCodecPtr getVideoCodec() const;
    void setVideoCodec(const VideoCodecPtr& codec);

    AudioCodecPtr getAudioCodec() const;
    void setAudioCodec(const AudioCodecPtr& codec);

    int checkCodec(const AVCodecID& id) const;       ///< \return Result of avformat_query_codec for the current format and given codec
    AVFormatContext* getContext() const;    ///< \return AVFormatContext to be used for rendering

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    wxString mName;
    wxString mLongName;
    wxStrings mExtensions;
    AVOutputFormat* mFormat; ///< Cloned variant of the associated ffmpeg output format. Cloned to avoid messing up ffmpeg internal adminstration when changing its video_codec/audio_codec
    AVCodecID mDefaultAudioCodec;
    AVCodecID mDefaultVideoCodec;

    VideoCodecPtr mVideoCodec;
    AudioCodecPtr mAudioCodec;

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<<(std::ostream& os, const OutputFormat& obj);

    //////////////////////////////////////////////////////////////////////////
    // SERIALIZATION
    //////////////////////////////////////////////////////////////////////////

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);

};

}} // namespace
