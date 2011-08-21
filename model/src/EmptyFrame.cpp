#include "EmptyFrame.h"
extern "C" {
#include <avformat.h>
};
#include "UtilInitAvcodec.h"

namespace model {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

EmptyFrame::EmptyFrame(VideoFrameType type, int width, int height, pts position)
:   VideoFrame(type, width, height, position)
,   mInitialized(false)
{
}

EmptyFrame::~EmptyFrame()
{
    // Deallocation handled by base class
}

//////////////////////////////////////////////////////////////////////////
// DATA ACCESS
//////////////////////////////////////////////////////////////////////////

DataPointer EmptyFrame::getData()
{
    if (!mInitialized)
    {
        PixelFormat format = mType == videoRGB ? PIX_FMT_RGB24 : PIX_FMT_RGBA;
        mBufferSize = avpicture_get_size(format, mWidth, mHeight);
        mBuffer = static_cast<boost::uint8_t*>(av_malloc(mBufferSize * sizeof(uint8_t)));

        mFrame = avcodec_alloc_frame();

        // Assign appropriate parts of buffer to image planes in mFrame
        avpicture_fill(reinterpret_cast<AVPicture*>(mFrame), mBuffer, format, mWidth, mHeight);
        mInitialized = true;
    }
    return VideoFrame::getData();
}

} // namespace

