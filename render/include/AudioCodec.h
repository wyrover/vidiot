#ifndef RENDER_AUDIO_CODEC_H
#define RENDER_AUDIO_CODEC_H

extern "C" {
#pragma warning(disable:4244)
#include <libavcodec/avcodec.h>
#pragma warning(default:4244)
}

#include <list>
#include <map>
#include <boost/shared_ptr.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/version.hpp>
#include <ostream>
#include "UtilCloneable.h"

namespace model { namespace render {

struct ICodecParameter;
typedef boost::shared_ptr<ICodecParameter> ICodecParameterPtr;
class AudioCodec;
typedef boost::shared_ptr<AudioCodec> AudioCodecPtr;

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
    std::list<ICodecParameterPtr> getParameters();
    void setParameters( AVCodecContext* codec ) const;

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    CodecID mId;
    std::list<ICodecParameterPtr> mParameters;

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