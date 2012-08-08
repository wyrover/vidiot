#ifndef RENDER_AUDIO_CODEC_H
#define RENDER_AUDIO_CODEC_H

extern "C" {
#pragma warning(disable:4244)
#include <libavcodec/avcodec.h>
#pragma warning(default:4244)
}

#include "UtilCloneable.h"

struct AVStream;
struct AVFormatContext;

namespace model { namespace render {

class AudioCodec
    :   public ICloneable
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    AudioCodec();
    explicit AudioCodec(CodecID id);
    AudioCodec(const AudioCodec& other);
	virtual ~AudioCodec();
    virtual AudioCodec* clone() const;

    //////////////////////////////////////////////////////////////////////////
    // OPERATORS
    //////////////////////////////////////////////////////////////////////////

    bool operator== (const AudioCodec& other) const;

    //////////////////////////////////////////////////////////////////////////
    // PARAMETERS
    //////////////////////////////////////////////////////////////////////////

    CodecID getId() const;
    AudioCodec& addParameter(ICodecParameter& parameter);
    ICodecParameters getParameters();
    AVStream* addStream(AVFormatContext* context) const; ///< Add a stream to the given libavformat format
    void open(AVCodecContext* context) const;            ///< Open the codec in libavcodec

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    CodecID mId;
    ICodecParameters mParameters;

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<<( std::ostream& os, const AudioCodec& obj );

    //////////////////////////////////////////////////////////////////////////
    // SERIALIZATION
    //////////////////////////////////////////////////////////////////////////

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);

};

}} // namespace

BOOST_CLASS_VERSION(model::render::AudioCodec, 1)

#endif // RENDER_AUDIO_CODEC_H