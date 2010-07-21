#ifndef EMPTY_FRAME_H
#define EMPTY_FRAME_H

#include "VideoFrame.h"

class EmptyFrame : public VideoFrame
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    EmptyFrame(PixelFormat format, int width, int height, boost::int64_t pts, AVRational timebase);
    
    virtual ~EmptyFrame();

    //////////////////////////////////////////////////////////////////////////
    // DATA ACCESS
    //////////////////////////////////////////////////////////////////////////

    /**
     * Override from VideoFrame. When this method is called for the first time,
     * the zeroed buffer is generated.
     */
    DataPointer getData();

private:
    
    bool mInitialized;
};

typedef boost::shared_ptr<EmptyFrame> EmptyFramePtr;

#endif // EMPTY_FRAME_H