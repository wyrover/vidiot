#ifndef RENDER_OUTPUT_FORMAT_H
#define RENDER_OUTPUT_FORMAT_H

#include "UtilCloneable.h"

namespace model { namespace render {

class OutputFormat
    :   public Cloneable<OutputFormat>
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    OutputFormat();
    explicit OutputFormat(wxString name, wxString longname, std::list<wxString> extensions, CodecID defaultaudiocodec, CodecID defaultvideocodec);
    OutputFormat(const OutputFormat& other);
    virtual ~OutputFormat();

    //////////////////////////////////////////////////////////////////////////
    // OPERATORS
    //////////////////////////////////////////////////////////////////////////

    bool operator== (const OutputFormat& other) const;

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    OutputFormat& setName(wxString name);
    wxString getName() const;
    wxString getLongName() const;
    std::list<wxString> getExtensions() const;
    CodecID getDefaultAudioCodec() const;
    CodecID getDefaultVideoCodec() const;

    bool storeAudio() const; ///< \return true if audio output is requested
    bool storeVideo() const; ///< \return true if video output is requested

    VideoCodecPtr getVideoCodec() const;
    void setVideoCodec(VideoCodecPtr codec);

    AudioCodecPtr getAudioCodec() const;
    void setAudioCodec(AudioCodecPtr codec);

    int checkCodec(CodecID id) const;       ///< \return Result of avformat_query_codec for the current format and given codec
    AVFormatContext* getContext() const;    ///< \return AVFormatContext to be used for rendering

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    wxString mName;
    wxString mLongName;
    std::list<wxString> mExtensions;
    CodecID mDefaultAudioCodec;
    CodecID mDefaultVideoCodec;

    VideoCodecPtr mVideoCodec;
    AudioCodecPtr mAudioCodec;

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<<( std::ostream& os, const OutputFormat& obj );

    //////////////////////////////////////////////////////////////////////////
    // SERIALIZATION
    //////////////////////////////////////////////////////////////////////////

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);

};

}} // namespace

#endif // RENDER_OUTPUT_FORMAT_H