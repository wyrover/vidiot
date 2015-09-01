// Copyright 2013-2015 Eric Raijmakers.
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

#include "VideoTransitionOpacity.h"

#include "UtilInt.h"
#include "UtilLog.h"
#include "VideoClip.h"
#include "VideoCompositionParameters.h"
#include "VideoFrameLayer.h"
#include "VideoSkipFrame.h"

namespace model {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

VideoTransitionOpacity::VideoTransitionOpacity()
    :	VideoTransition()
{
    VAR_DEBUG(this);
}

VideoTransitionOpacity::VideoTransitionOpacity(const VideoTransitionOpacity& other)
    :   VideoTransition(other)
{
    VAR_DEBUG(*this);
}

VideoTransitionOpacity::~VideoTransitionOpacity()
{
    VAR_DEBUG(this);
}

//////////////////////////////////////////////////////////////////////////
// IMPLEMENTATION OF TRANSITION
//////////////////////////////////////////////////////////////////////////

VideoFramePtr VideoTransitionOpacity::getVideo(pts position, const IClipPtr& leftClip, const IClipPtr& rightClip, const VideoCompositionParameters& parameters)
{
    VAR_DEBUG(position)(parameters);
    VideoFramePtr result = parameters.getSkip() ? boost::make_shared<VideoSkipFrame>(parameters) : boost::make_shared<VideoFrame>(parameters);

    auto applyStep = [this, result, parameters, position](IClipPtr clip, bool left)
    {
        if (clip)
        {
            VideoFramePtr frame = boost::static_pointer_cast<VideoClip>(clip)->getNextVideo(parameters);
            if (parameters.getSkip())
            {
                // Just a getNextVideo is all that's required
            }
            else
            {
                if (frame)
                {
                    for (auto layer : frame->getLayers() )
                    {
                        if (layer)
                        {
                            wxImagePtr image = layer->getImage();
                            float factor = (float)position / (float)getLength();
                            if (image)
                            {
                                std::function<float (int,int)> f =
                                    left ? getLeftMethod(image, factor) : getRightMethod(image, factor);
                                if (image->HasAlpha())
                                {
                                    handleImageWithAlpha(image, f);
                                }
                                else
                                {
                                    image->InitAlpha();
                                    handleFullyOpaqueImage(image, f);
                                }
                                result->addLayer(layer);
                            }
                        }
                    }
                }
            }
        }

    };

    applyStep(leftClip, true);
    applyStep(rightClip, false);

    return result;
};

void VideoTransitionOpacity::applyToAllPixels(const wxImagePtr& image, const std::function<float (int, int)>& f) const
{
    // Each pixel done separately (existing alpha may be != max opacity)
    int opacityLineWidth = image->GetWidth();

    unsigned char* opacityBegin = image->GetAlpha();
    for (int y = 0; y < image->GetSize().GetHeight(); ++y)
    {
        unsigned char* opacity = opacityBegin + y * opacityLineWidth;
        for (int x = 0; x < image->GetSize().GetWidth(); ++x)
        {
            *opacity = *opacity * f(x,y);
            ++opacity;
            // NOT: *opacity++ = *opacity * f(x,y);
            // Gives problems (on linux/GCC) because operand 'opacity' is used twice in the expression,
            // see http://en.wikipedia.org/wiki/Increment_and_decrement_operators
        }
    }
}

void VideoTransitionOpacity::applyToFirstLineThenCopy(const wxImagePtr& image, const std::function<float (int, int)>& f) const
{
    // Optimized for speed. Do one line, then copy that line.
    int opacityLineWidth = image->GetWidth();

    unsigned char* opacityBegin = image->GetAlpha();
    unsigned char* opacity = opacityBegin;
    // First line
    for (int x = 0; x < image->GetSize().GetWidth(); ++x)
    {
        *opacity = *opacity * f(x,0);
        ++opacity;
        // NOT: *opacity++ = *opacity * f(x,y);
        // Gives problems (on linux/GCC) because operand 'opacity' is used twice in the expression,
        // see http://en.wikipedia.org/wiki/Increment_and_decrement_operators
    }
    // Then other lines
    for (int y = 1; y < image->GetSize().GetHeight(); ++y)
    {
        memcpy(opacityBegin + y * opacityLineWidth, opacityBegin, opacityLineWidth);
    }
}

} //namespace
