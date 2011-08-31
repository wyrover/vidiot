#include "CrossFade.h"

#include <boost/make_shared.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include "UtilLog.h"
#include "VideoClip.h"
#include "VideoFrame.h"

namespace model { namespace transition {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

CrossFade::CrossFade()
    :	VideoTransition()
{
    VAR_DEBUG(this);
}

CrossFade::CrossFade(pts nFramesLeft, pts nFramesRight)
    :   VideoTransition(nFramesLeft, nFramesRight)
{
    VAR_DEBUG(this);
}

CrossFade::CrossFade(const CrossFade& other)
    :   VideoTransition(other)
{
    VAR_DEBUG(*this);
}

CrossFade* CrossFade::clone()
{ 
    return new CrossFade(static_cast<const CrossFade&>(*this)); 
}

CrossFade::~CrossFade()
{
    VAR_DEBUG(this);
}

//////////////////////////////////////////////////////////////////////////
// IVIDEO
//////////////////////////////////////////////////////////////////////////

VideoFramePtr CrossFade::getVideo(pts position, IClipPtr leftClip, IClipPtr rightClip, int requestedWidth, int requestedHeight, bool alpha)
{
    VideoFramePtr leftFrame   = leftClip  ? boost::static_pointer_cast<VideoClip>(leftClip)->getNextVideo(requestedWidth,requestedHeight,alpha)  : VideoFramePtr();
    VideoFramePtr rightFrame  = rightClip ? boost::static_pointer_cast<VideoClip>(rightClip)->getNextVideo(requestedWidth,requestedHeight,alpha) : VideoFramePtr();
    VideoFramePtr targetFrame =             boost::make_shared<VideoFrame>(alpha ? videoRGBA : videoRGB, requestedWidth, requestedHeight, 1, 1);
    VAR_DEBUG(position)(requestedWidth)(requestedHeight)(alpha)(leftFrame)(rightFrame)(targetFrame);

    pts steps = getLength();
    float factorLeft = ((float)getLength() - (float)position) / (float)getLength();
    float factorRight = (float)position / (float)getLength();
    VAR_DEBUG(factorLeft)(factorRight);

    unsigned char* leftData   = leftFrame  ? leftFrame  ->getData()[0] : 0;
    unsigned char* rightData  = rightFrame ? rightFrame ->getData()[0] : 0;
    unsigned char* targetData =              targetFrame->getData()[0];

    int leftBytesPerLine   = leftFrame  ? leftFrame  ->getLineSizes()[0] : 0;
    int rightBytesPerLine  = rightFrame ? rightFrame ->getLineSizes()[0] : 0;
    int targetBytesPerLine =              targetFrame->getLineSizes()[0];

    VAR_DEBUG(leftBytesPerLine)(rightBytesPerLine)(targetBytesPerLine);

    int bytesPerPixel = alpha ? 4 : 3;

    for (int y = 0; y < targetFrame->getHeight(); ++y)
    {
        for (int x = 0; x < targetFrame->getWidth() * bytesPerPixel; x += 1)
        {
            unsigned char left = 0;
            if (leftFrame && y < leftFrame->getHeight() && x < leftFrame->getWidth() * bytesPerPixel)
            {
                left = *(leftData + y * leftBytesPerLine + x);
            }
            unsigned char right = 0;
            if (rightFrame && y < rightFrame->getHeight() && x < rightFrame->getWidth() * bytesPerPixel)
            {
                right = *(rightData + y * rightBytesPerLine + x);
            }
            *(targetData + y * targetBytesPerLine + x) = (unsigned char)(left * factorLeft + right * factorRight);
        }
    }

    return targetFrame;
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<( std::ostream& os, const CrossFade& obj )
{
    os << static_cast<const VideoTransition&>(obj);
    return os;
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION 
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void CrossFade::serialize(Archive & ar, const unsigned int version)
{
    ar & boost::serialization::base_object<VideoTransition>(*this);
}
template void CrossFade::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void CrossFade::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);

}} //namespace

