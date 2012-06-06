#ifndef VIDEO_FRAME_COMPOSITION_H
#define VIDEO_FRAME_COMPOSITION_H

#include <wx/gdicmn.h>
#include <list>
#include <boost/shared_ptr.hpp>
#include "VideoParameters.h"

namespace model {

class VideoFrame;
typedef boost::shared_ptr<VideoFrame> VideoFramePtr;
typedef std::list<VideoFramePtr> VideoFrames;
class VideoParameters;
class VideoFrameComposition;
typedef boost::shared_ptr<VideoFrameComposition> VideoFrameCompositionPtr;

class VideoFrameComposition // todo rename to videocomposition
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    VideoFrameComposition(const model::VideoParameters& parameters);

    VideoFrameComposition(const VideoFrameComposition& other);

    virtual ~VideoFrameComposition();

    //////////////////////////////////////////////////////////////////////////
    // COMPOSITION
    //////////////////////////////////////////////////////////////////////////

    void add(VideoFramePtr frame);
    void replace(VideoFramePtr oldFrame, VideoFramePtr newFrame);

    /// Render the composition
    /// \return composition of all input frames.
    /// \note may return '0' to indicate that the composition is completely empty.
    /// \note the pts position value of the returned frame is always 0
    VideoFramePtr generate();

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    VideoParameters getParameters() const;

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    VideoParameters mParameters;
    VideoFrames mFrames;
    wxSize mBoundingBox;

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<<( std::ostream& os, const VideoFrameComposition& obj );

};

} // namespace

#endif // VIDEO_FRAME_COMPOSITION_H