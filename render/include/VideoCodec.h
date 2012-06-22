#ifndef RENDER_VIDEO_CODEC_H
#define RENDER_VIDEO_CODEC_H

extern "C" {
#pragma warning(disable:4244)
#include <avcodec.h>
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
class VideoCodec;
typedef boost::shared_ptr<VideoCodec> VideoCodecPtr;

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
    // PARAMETERS
    //////////////////////////////////////////////////////////////////////////

    CodecID getId() const;
    VideoCodec& addParameter(ICodecParameter& parameter);
        std::list<ICodecParameterPtr> getParameters() { return mParameters; }

    //video_codec->max_b_frames = 2; // just for testing, we also add B frames

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