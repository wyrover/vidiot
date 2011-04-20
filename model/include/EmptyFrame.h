#ifndef EMPTY_FRAME_H
#define EMPTY_FRAME_H

#include "VideoFrame.h"

namespace model {

class EmptyFrame;
typedef boost::shared_ptr<EmptyFrame> EmptyFramePtr;

class EmptyFrame 
    :   public VideoFrame
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    EmptyFrame(VideoFrameType type, int width, int height, pts position);

    virtual ~EmptyFrame();

    //////////////////////////////////////////////////////////////////////////
    // DATA ACCESS
    //////////////////////////////////////////////////////////////////////////

    /// Override from VideoFrame. When this method is called for the first time,
    /// the zeroed buffer is generated.
    DataPointer getData();

private:

    bool mInitialized;
};

} // namespace

#endif // EMPTY_FRAME_H