#include "VideoTransition.h"

#include <boost/make_shared.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include "UtilInt.h"
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
    VideoFramePtr targetFrame = boost::make_shared<VideoFrame>(alpha ? videoRGBA : videoRGB, requestedWidth, requestedHeight, 1, 1);

    pts steps = getLength();
    float factorLeft = ((float)getLength() - (float)mPosition) / (float)getLength();
    float factorRight = (float)mPosition / (float)getLength();
    VAR_DEBUG(factorLeft)(factorRight);

    unsigned char* leftData = leftFrame->getData()[0];
    unsigned char* rightData = rightFrame->getData()[0];
    unsigned char* targetData = targetFrame->getData()[0];

    int leftBytesPerLine = leftFrame->getLineSizes()[0];
    int rightBytesPerLine = rightFrame->getLineSizes()[0];
    int targetBytesPerLine = targetFrame->getLineSizes()[0];

    int bytesPerPixel = alpha ? 4 : 3;

    for (int y = 0; y < targetFrame->getHeight(); ++y)
    {
        for (int x = 0; x < targetFrame->getWidth() * bytesPerPixel; x += 1) // todo 3 vs 4 for alpha
        {
            unsigned char left = 0;
            if (y < leftFrame->getHeight() && x < leftFrame->getWidth() * bytesPerPixel)
            {
                left = *(leftData + y * leftBytesPerLine + x);
            }
            unsigned char right = 0;
            if (y < rightFrame->getHeight() && x < rightFrame->getWidth() * bytesPerPixel)
            {
                right = *(rightData + y * rightBytesPerLine + x);
            }
            unsigned char* target = targetData + y * targetBytesPerLine + x;
            target[0] = (unsigned char)(left * factorLeft + right * factorRight);
        }
    }

    mPosition++;

    return targetFrame;
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

