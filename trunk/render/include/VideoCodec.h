#ifndef RENDER_VIDEO_CODEC_H
#define RENDER_VIDEO_CODEC_H

extern "C" {
#pragma warning(disable:4244)
#include <libavcodec/avcodec.h>
#pragma warning(default:4244)
}

#include <boost/serialization/access.hpp>
#include <boost/serialization/version.hpp>
#include <ostream>
#include "UtilCloneable.h"

namespace model { namespace render {

class VideoCodec
    :   public ICloneable
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    VideoCodec();

    explicit VideoCodec(CodecID id);
    VideoCodec(const VideoCodec& other);
	virtual ~VideoCodec();
    virtual VideoCodec* clone() const override;

    //////////////////////////////////////////////////////////////////////////
    // OPERATORS
    //////////////////////////////////////////////////////////////////////////

    bool operator== (const VideoCodec& other) const;

    //////////////////////////////////////////////////////////////////////////
    // PARAMETERS
    //////////////////////////////////////////////////////////////////////////

    CodecID getId() const;
    VideoCodec& addParameter(ICodecParameter& parameter);
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

    friend std::ostream& operator<<( std::ostream& os, const VideoCodec& obj );

    //////////////////////////////////////////////////////////////////////////
    // SERIALIZATION
    //////////////////////////////////////////////////////////////////////////

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);

};

}} // namespace

BOOST_CLASS_VERSION(model::render::VideoCodec, 1)

#endif // RENDER_VIDEO_CODEC_H