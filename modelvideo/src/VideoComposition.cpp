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

#include "VideoComposition.h"

#include "Constants.h"
#include "Convert.h"
#include "EmptyFrame.h"
#include "Properties.h"
#include "UtilList.h"
#include "UtilLogStl.h"
#include "UtilLogWxwidgets.h"
#include "VideoFrame.h"
#include "VideoFrameLayer.h"
#include "VideoCompositionParameters.h"

namespace model {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

VideoComposition::VideoComposition(const VideoCompositionParameters& parameters)
    :   mFrames()
    ,   mParameters(parameters)
{
    VAR_DEBUG(this);
}

VideoComposition::~VideoComposition()
{
    VAR_DEBUG(this);
}

//////////////////////////////////////////////////////////////////////////
// COMPOSITION
//////////////////////////////////////////////////////////////////////////

void VideoComposition::add(VideoFramePtr frame)
{
    if (!frame || frame->isA<EmptyFrame>())
    {
        // Skip empty frames.
        return;
    }
    ASSERT_EQUALS(frame->getParameters().getBoundingBox(), mParameters.getBoundingBox());
    mFrames.push_back(frame);
}

VideoFramePtr VideoComposition::generate()
{
    if (mFrames.empty())
    {
        return boost::make_shared<EmptyFrame>(mParameters);
    }

    VideoFramePtr result = boost::make_shared<VideoFrame>(mParameters);
    bool keyFrame = false;
    VideoFrameLayers layers;
    for ( auto frame : mFrames )
    {
        keyFrame = keyFrame || frame->getForceKeyFrame();
        for ( auto layer : frame->getLayers() )
        {
            result->addLayer(layer);
        }
    }
    result->setForceKeyFrame(keyFrame);
    return result;
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

VideoCompositionParameters VideoComposition::getParameters() const
{
    return mParameters;
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<( std::ostream& os, const VideoComposition& obj )
{
    os << &obj << '|' << obj.mParameters << '|' << obj.mFrames;
    return os;
}

} //namespace