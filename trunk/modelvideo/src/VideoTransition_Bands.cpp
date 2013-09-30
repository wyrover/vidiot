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

#include "VideoTransition_Bands.h"

#include "TransitionFactory.h"
#include "UtilLog.h"
#include "UtilLogWxwidgets.h"
#include "VideoClip.h"
#include "VideoCompositionParameters.h"
#include "VideoFrame.h"

namespace model { namespace video { namespace transition {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

Bands::Bands()
    :	VideoTransition()
{
    VAR_DEBUG(this);
}

Bands::Bands(const Bands& other)
    :   VideoTransition(other)
{
    VAR_DEBUG(*this);
}

Bands* Bands::clone() const
{
    return new Bands(static_cast<const Bands&>(*this));
}

Bands::~Bands()
{
    VAR_DEBUG(this);
}

//////////////////////////////////////////////////////////////////////////
// IVIDEO
//////////////////////////////////////////////////////////////////////////

VideoFramePtr Bands::getVideo(pts position, IClipPtr leftClip, IClipPtr rightClip, const VideoCompositionParameters& parameters)
{
    VideoFramePtr leftFrame   = leftClip  ? boost::static_pointer_cast<VideoClip>(leftClip)->getNextVideo(parameters)  : VideoFramePtr();
    VideoFramePtr rightFrame  = rightClip ? boost::static_pointer_cast<VideoClip>(rightClip)->getNextVideo(parameters) : VideoFramePtr();
    VAR_DEBUG(position)(parameters)(leftFrame)(rightFrame);

    pts steps = getLength();

    // todo make mechanism for changing parameters of transitions (left-to-right vs right-to-left, etc, but also number of bands)
    // todo make 'Details' view for transitions (allows changing parameters and changing transition type)
    int nBands = 10;

    float factorLeft = ((float)getLength() - (float)position) / (float)getLength();
    float factorRight = (float)position / (float)getLength();

    int bandSize = parameters.getBoundingBox().x / nBands;
    std::vector<float> multiplier(bandSize + 1,0.0); // +1 for rounding diffs

    int lastLeftPixelShown = factorLeft * bandSize;
    int firstRightPixel = factorRight * bandSize;
    for (int i = 0; i <= bandSize; ++i)
    {
        if (i > firstRightPixel)
        {
            multiplier[i] = 1.0;
        }
        else
        {
            multiplier[i] = 0.0;
        }
    }

    VideoFramePtr targetFrame = boost::make_shared<VideoFrame>(parameters.getBoundingBox(), 1, true); // todo is the pts param ever diff than 1?

    unsigned char* leftData   = leftFrame  ? leftFrame  ->getData()[0] : 0;
    unsigned char* rightData  = rightFrame ? rightFrame ->getData()[0] : 0;
    unsigned char* targetData =              targetFrame->getData()[0];

    int leftBytesPerLine   = leftFrame  ? leftFrame  ->getLineSizes()[0] : 0;
    int rightBytesPerLine  = rightFrame ? rightFrame ->getLineSizes()[0] : 0;
    int targetBytesPerLine =              targetFrame->getLineSizes()[0];

    VAR_DEBUG(leftBytesPerLine)(rightBytesPerLine)(targetBytesPerLine);

    int bytesPerPixel = 3;
    for (int y = 0; y < targetFrame->getSize().GetHeight(); ++y)
    {
        for (int x = 0; x < targetFrame->getSize().GetWidth(); x += 1)
        {
            float factorLeftPixel = multiplier[x % bandSize];
            float factorRightPixel = 1 - factorLeftPixel;

            for (int byte = 0; byte < bytesPerPixel; ++byte)
            {
                unsigned char left = 0;
                if (leftFrame &&
                    y < leftFrame->getSize().GetHeight() &&
                    x < leftFrame->getSize().GetWidth())
                {
                    left = *(leftData + y * leftBytesPerLine + x * bytesPerPixel + byte);
                }
                unsigned char right = 0;
                if (rightFrame &&
                    y < rightFrame->getSize().GetHeight() &&
                    x < rightFrame->getSize().GetWidth())
                {
                    right = *(rightData + y * rightBytesPerLine + x * bytesPerPixel + byte);
                }
                *(targetData + y * targetBytesPerLine + x * bytesPerPixel + byte) =
                    (unsigned char)(left * factorLeftPixel + right * factorRightPixel);
            }
        }
    }

    return targetFrame;
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<( std::ostream& os, const Bands& obj )
{
    os << static_cast<const VideoTransition&>(obj);
    return os;
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void Bands::serialize(Archive & ar, const unsigned int version)
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