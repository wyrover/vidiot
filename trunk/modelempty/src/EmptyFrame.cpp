#include "EmptyFrame.h"

#include "UtilInitAvcodec.h"

namespace model {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

EmptyFrame::EmptyFrame(wxSize size, pts position)
:   VideoFrame(size, position)
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
        PixelFormat format = PIX_FMT_RGB24;
        mBufferSize = avpicture_get_size(format, getSize().GetWidth(), getSize().GetHeight());
        mBuffer = static_cast<boost::uint8_t*>(av_malloc(mBufferSize * sizeof(uint8_t)));

        mFrame = avcodec_alloc_frame();

        // Assign appropriate parts of buffer to image planes in mFrame
        avpicture_fill(reinterpret_cast<AVPicture*>(mFrame), mBuffer, format,getSize().GetWidth(), getSize().GetHeight());
        mInitialized = true;
    }
    return VideoFrame::getData();
}

} // namespace