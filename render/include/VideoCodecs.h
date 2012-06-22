#ifndef RENDER_VIDEO_CODECS_H
#define RENDER_VIDEO_CODECS_H

extern "C" {
#pragma warning(disable:4244)
#include <avcodec.h>
#pragma warning(default:4244)
}

#include <map>
#include <boost/shared_ptr.hpp>

namespace model { namespace render {

struct ICodecParameter;
typedef boost::shared_ptr<ICodecParameter> ICodecParameterPtr;
class VideoCodec;
typedef boost::shared_ptr<VideoCodec> VideoCodecPtr;

typedef std::map<CodecID,VideoCodecPtr> VideoCodecMap;

class VideoCodecs
{
public:
    static void initialize();
    static void add(VideoCodec codec);

    /// \return 0 if a codec with the given id was not found
    /// This method finds the given codec in the registered list of codecs
    static VideoCodecPtr find(CodecID id);
private:
    static VideoCodecMap sVideoCodecs;
};

}} // namespace

#endif // RENDER_VIDEO_CODECS_H