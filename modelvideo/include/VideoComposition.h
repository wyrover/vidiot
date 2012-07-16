#ifndef VIDEO_FRAME_COMPOSITION_H
#define VIDEO_FRAME_COMPOSITION_H

#include <wx/gdicmn.h>
#include <list>
#include <boost/shared_ptr.hpp>
#include "VideoCompositionParameters.h"

namespace model {

class VideoFrame;
typedef boost::shared_ptr<VideoFrame> VideoFramePtr;
typedef std::list<VideoFramePtr> VideoFrames;
class VideoCompositionParameters;
class VideoComposition;
typedef boost::shared_ptr<VideoComposition> VideoCompositionPtr;

class VideoComposition
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    VideoComposition(const model::VideoCompositionParameters& parameters);
    VideoComposition(const VideoComposition& other);
    virtual ~VideoComposition();

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

    VideoCompositionParameters getParameters() const;

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    VideoCompositionParameters mParameters;
    VideoFrames mFrames;
    wxSize mBoundingBox;

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<<( std::ostream& os, const VideoComposition& obj );

};

} // namespace

#endif // VIDEO_FRAME_COMPOSITION_H