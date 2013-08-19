// Copyright 2013 Eric Raijmakers.
//
// This file is part of Vidiot.
//
// Vidiot is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Vidiot is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Vidiot. If not, see <http://www.gnu.org/licenses/>.

#include "VideoTransition_CrossFade.h"

#include "TransitionFactory.h"
#include "UtilLog.h"
#include "UtilLogWxwidgets.h"
#include "VideoClip.h"
#include "VideoComposition.h"
#include "VideoCompositionParameters.h"
#include "VideoFrame.h"

namespace model { namespace video { namespace transition {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

CrossFade::CrossFade()
    :	VideoTransition()
{
    VAR_DEBUG(this);
}

CrossFade::CrossFade(const CrossFade& other)
    :   VideoTransition(other)
{
    VAR_DEBUG(*this);
}

CrossFade* CrossFade::clone() const
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

VideoFramePtr CrossFade::getVideo(pts position, IClipPtr leftClip, IClipPtr rightClip, const VideoCompositionParameters& parameters)
{
    VideoComposition composition(VideoCompositionParameters(parameters).setDrawBoundingBox(false));

    VideoFramePtr leftFrame   = leftClip  ? boost::static_pointer_cast<VideoClip>(leftClip)->getNextVideo(parameters)  : VideoFramePtr();
    VideoFramePtr rightFrame  = rightClip ? boost::static_pointer_cast<VideoClip>(rightClip)->getNextVideo(parameters) : VideoFramePtr();
    VAR_DEBUG(position)(parameters)(leftFrame)(rightFrame);

    pts steps = getLength();

    if (leftFrame)
    {
        float factorLeft = ((float)getLength() - (float)position) / (float)getLength();
        int opacityLeft = factorLeft * leftFrame->getOpacity();
        leftFrame->setOpacity(opacityLeft);
        VAR_DEBUG(factorLeft)(leftFrame->getOpacity())(opacityLeft);
        composition.add(leftFrame);
    }

    if (rightFrame)
    {
        float factorRight = (float)position / (float)getLength();
        int opacityRight = factorRight * rightFrame->getOpacity();
        rightFrame->setOpacity(opacityRight);
        VAR_DEBUG(factorRight)(rightFrame->getOpacity())(opacityRight);
        composition.add(rightFrame);
    }

    return composition.generate();

    // Old code temporarily kept for reference:
    //VideoFramePtr targetFrame =             boost::make_shared<VideoFrame>(parameters.getBoundingBox(), 1, 1);

    //unsigned char* leftData   = leftFrame  ? leftFrame  ->getData()[0] : 0;
    //unsigned char* rightData  = rightFrame ? rightFrame ->getData()[0] : 0;
    //unsigned char* targetData =              targetFrame->getData()[0];

    //int leftBytesPerLine   = leftFrame  ? leftFrame  ->getLineSizes()[0] : 0;
    //int rightBytesPerLine  = rightFrame ? rightFrame ->getLineSizes()[0] : 0;
    //int targetBytesPerLine =              targetFrame->getLineSizes()[0];

    //VAR_DEBUG(leftBytesPerLine)(rightBytesPerLine)(targetBytesPerLine);

    //int bytesPerPixel = 3;
    //for (int y = 0; y < targetFrame->getSize().GetHeight(); ++y)
    //{
    //    for (int x = 0; x < targetFrame->getSize().GetWidth() * bytesPerPixel; x += 1)
    //    {
    //        unsigned char left = 0;
    //        if (leftFrame && y < leftFrame->getSize().GetHeight() && x < leftFrame->getSize().GetWidth() * bytesPerPixel)
    //        {
    //            left = *(leftData + y * leftBytesPerLine + x);
    //        }
    //        unsigned char right = 0;
    //        if (rightFrame && y < rightFrame->getSize().GetHeight() && x < rightFrame->getSize().GetWidth() * bytesPerPixel)
    //        {
    //            right = *(rightData + y * rightBytesPerLine + x);
    //        }
    //        *(targetData + y * targetBytesPerLine + x) = (unsigned char)(left * factorLeft + right * factorRight);
    //    }
    //}
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
    try
    {
        ar & boost::serialization::base_object<VideoTransition>(*this);
    }
    catch (boost::archive::archive_exception& e) { VAR_ERROR(e.what());                         throw; }
    catch (boost::exception &e)                  { VAR_ERROR(boost::diagnostic_information(e)); throw; }
    catch (std::exception& e)                    { VAR_ERROR(e.what());                         throw; }
    catch (...)                                  { LOG_ERROR;                                   throw; }
}
template void CrossFade::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void CrossFade::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);

}}} //namespace