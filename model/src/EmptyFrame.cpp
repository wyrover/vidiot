#include "EmptyFrame.h"
#include "UtilLogAvcodec.h"

namespace model {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

EmptyFrame::EmptyFrame(PixelFormat format, int width, int height, pts position, AVRational timebase)
:   VideoFrame(format, width, height, position, timebase)
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
        mBufferSize = avpicture_get_size(mFormat, mWidth, mHeight);
        mBuffer = static_cast<boost::uint8_t*>(av_malloc(mBufferSize * sizeof(uint8_t)));

        mFrame = avcodec_alloc_frame();

        // Assign appropriate parts of buffer to image planes in mFrame
        avpicture_fill(reinterpret_cast<AVPicture*>(mFrame), mBuffer, mFormat, mWidth, mHeight);
        mInitialized = true;
    }
    return VideoFrame::getData();
}

} // namespace

