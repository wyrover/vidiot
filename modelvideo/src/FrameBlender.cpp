#include "FrameBlender.h"

#include <boost/foreach.hpp>
#include "EmptyFrame.h"

namespace model {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

FrameBlender::FrameBlender()
    :   mFrames()
{
    VAR_DEBUG(this);
}

FrameBlender::~FrameBlender()
{
    VAR_DEBUG(this);
}

//////////////////////////////////////////////////////////////////////////
// ADD TRACKS TO BE BLENDED
//////////////////////////////////////////////////////////////////////////

void FrameBlender::add(VideoFramePtr frame)
{
    if (frame && !frame->isA<EmptyFrame>())
    {
        // Skip empty frames.
        mFrames.push_back(frame);
    }
}

//////////////////////////////////////////////////////////////////////////
// BLEND
//////////////////////////////////////////////////////////////////////////

VideoFramePtr FrameBlender::blend()
{
    if (mFrames.empty())
    {
        return EmptyFramePtr();
    }
    VideoFramePtr result;
    BOOST_REVERSE_FOREACH( VideoFramePtr frame, mFrames )
    {
        // todo add alpha handling, particularly when one of the upper track's clips has been reduced in size.
        // From the top track, the first found frame is returned.
        result = frame;
        break; // todo only stop if the current data is fully opaque?
    }
    return result;
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<( std::ostream& os, const FrameBlender& obj )
{
//    os << static_cast<const FrameBlender&>(obj);
    return os;
}

} //namespace