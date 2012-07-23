#ifndef EMPTY_FRAME_H
#define EMPTY_FRAME_H

#include "VideoFrame.h"

namespace model {

class EmptyFrame
    :   public VideoFrame
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    EmptyFrame(wxSize size, pts position);

    virtual ~EmptyFrame();

    //////////////////////////////////////////////////////////////////////////
    // DATA ACCESS
    //////////////////////////////////////////////////////////////////////////

    /// Override from VideoFrame. When this method is called for the first time,
    /// the zeroed buffer is generated.
    DataPointer getData() override;

private:

    bool mInitialized;
};

} // namespace

#endif // EMPTY_FRAME_H