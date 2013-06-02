#include "VideoTransition.h"

#include "UtilInt.h"
#include "UtilLog.h"
#include "VideoClip.h"
#include "VideoCompositionParameters.h"
#include "VideoFrame.h"

namespace model {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

VideoTransition::VideoTransition()
    :	Transition()
    ,   IVideo()
    ,   mProgress(-1)
    ,   mLeftClip()
    ,   mRightClip()
{
    VAR_DEBUG(this);
}

VideoTransition::VideoTransition(const VideoTransition& other)
    :   Transition(other)
    ,   IVideo()
    ,   mProgress(-1)
    ,   mLeftClip()
    ,   mRightClip()
{
    VAR_DEBUG(*this);
}

VideoTransition::~VideoTransition()
{
    VAR_DEBUG(this);
}

//////////////////////////////////////////////////////////////////////////
// IVIDEO
//////////////////////////////////////////////////////////////////////////

VideoFramePtr VideoTransition::getNextVideo(const VideoCompositionParameters& parameters)
{
    if (getNewStartPosition())
    {
        mProgress = *getNewStartPosition(); // Reinitialize mProgress to the last value set in ::moveTo
        invalidateNewStartPosition();

        // Note: When creating a transition, the left and right clip are adjusted (shortened) to
        // accomodate for the addition of the transition. Therefore, the computations below take these
        // shortened clips as input.

        if (getLeft() > 0)
        {
            ASSERT(getPrev());
            mLeftClip = makeLeftClip();
            mLeftClip->moveTo(mProgress);
        }
        if (getRight() > 0)
        {
            ASSERT(getNext());
            mRightClip = makeRightClip();
            mRightClip->moveTo(mProgress);
        }

        ASSERT(!mLeftClip || !mRightClip || mLeftClip->getLength() == mRightClip->getLength());
    }
    VideoFramePtr videoFrame;
    if (mProgress < getLength())
    {
        videoFrame = getVideo(mProgress, mLeftClip, mRightClip, parameters);
        mProgress++;
    }
    VAR_VIDEO(videoFrame);
    setGenerationProgress(mProgress);
    return videoFrame;
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<( std::ostream& os, const VideoTransition& obj )
{
    os << static_cast<const Transition&>(obj) << '|' << obj.mProgress;
    return os;
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void VideoTransition::serialize(Archive & ar, const unsigned int version)
{
    ar & boost::serialization::base_object<Transition>(*this);
    ar & boost::serialization::base_object<IVideo>(*this);
}
template void VideoTransition::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void VideoTransition::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);

} //namespace