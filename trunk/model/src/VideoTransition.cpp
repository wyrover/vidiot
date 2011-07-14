#include "VideoTransition.h"

#include <boost/make_shared.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include "UtilLog.h"
#include "VideoClip.h"
#include "VideoFrame.h"

namespace model {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

VideoTransition::VideoTransition()
    :	Transition()
    ,   IVideo()
    ,   mPosition(-1)
{
    VAR_DEBUG(this);
}

VideoTransition::VideoTransition(IClipPtr left, pts nFramesLeft, IClipPtr right, pts nFramesRight)
    :   Transition(left, nFramesLeft, right, nFramesRight)
    ,   IVideo()
    ,   mPosition(-1)
{
    VAR_DEBUG(this);
}

VideoTransition::VideoTransition(const VideoTransition& other)
    :   Transition(other)
    ,   IVideo()
    ,   mPosition(-1)
{
    VAR_DEBUG(*this);
}

VideoTransition* VideoTransition::clone()
{ 
    return new VideoTransition(static_cast<const VideoTransition&>(*this)); 
}

VideoTransition::~VideoTransition()
{
    VAR_DEBUG(this);
}

//////////////////////////////////////////////////////////////////////////
// IVIDEO
//////////////////////////////////////////////////////////////////////////

VideoFramePtr VideoTransition::getNextVideo(int requestedWidth, int requestedHeight, bool alpha)
{
    if (getLastSetPosition())
    {
        mPosition = *getLastSetPosition(); // Reinitialize mProgress to the last value set in ::moveTo
        invalidateLastSetPosition();

        getLeftClip()->moveTo(mPosition);
        getRightClip()->moveTo(mPosition);
    }

    VideoFramePtr leftFrame = boost::static_pointer_cast<VideoClip>(getLeftClip())->getNextVideo(requestedWidth,requestedHeight,alpha);
    VideoFramePtr rightFrame = boost::static_pointer_cast<VideoClip>(getRightClip())->getNextVideo(requestedWidth,requestedHeight,alpha);

    VideoFramePtr combined = boost::make_shared<VideoFrame>(alpha ? videoRGBA : videoRGB, requestedWidth, requestedHeight, 1, 1);

    pts steps = getLength();
    float factorLeft = ((float)getLength() - (float)mPosition) / (float)getLength();
    float factorRight = (float)mPosition / (float)getLength();
    VAR_DEBUG(factorLeft)(factorRight);

    for (int y = 0; y < leftFrame->getHeight(); ++y)
    {
        for (int x = 0; x < leftFrame->getWidth() * 3; x += 1) // todo 3 vs 4 for alpha
        {
            unsigned char* left = leftFrame->getData()[0] + y * leftFrame->getLineSizes()[0] + x;
            unsigned char* right = rightFrame->getData()[0] + y * rightFrame->getLineSizes()[0] + x;
            unsigned char* data = combined->getData()[0] + y * combined->getLineSizes()[0] + x;
            float result = *left * factorLeft + *right * factorRight;
            unsigned char c = (unsigned char)result;
            data[0] = c;
           // data[1] = left[1];
            //data[2] = left[2];
            //        data[i] = left[i];
        }
    }
    // todo fill area of transition that is unfilled

    mPosition++;

    return combined;
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<( std::ostream& os, const VideoTransition& obj )
{
    os << static_cast<const Transition&>(obj) << '|' << obj.mPosition;
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

