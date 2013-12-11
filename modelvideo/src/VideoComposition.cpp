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

VideoComposition::VideoComposition(const VideoComposition& other)
    :   mFrames(other.mFrames)
    ,   mParameters(other.mParameters)
{
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
    if (frame && !frame->isA<EmptyFrame>())
    {
        // Skip empty frames.
        mFrames.push_back(frame);
    }
}

void VideoComposition::replace(VideoFramePtr oldFrame, VideoFramePtr newFrame)
{
    UtilList<model::VideoFramePtr>(mFrames).replace(oldFrame,newFrame);
}

VideoFramePtr VideoComposition::generate()
{
    wxSize outputsize = Properties::get().getVideoSize();
    if (mFrames.empty())
    {
        return boost::make_shared<EmptyFrame>(outputsize);
    }

    if (mFrames.size() == 1)
    {
        VideoFramePtr front = mFrames.front();
        if (front->getOpacity() == Constants::sMaxOpacity && front->getPosition() == wxPoint(0,0))
        {
            // Performance optimization: if only one frame is rendered, return that frame, but only if the frame requires no 'processing'.
            return front;
        }
    }

    boost::rational<int> scaleToBoundingBox(0);
    wxSize requiredOutputSize = Convert::sizeInBoundingBox(outputsize, mParameters.getBoundingBox(), scaleToBoundingBox);
    ASSERT_NONZERO(scaleToBoundingBox);

    wxImagePtr compositeImage(boost::make_shared<wxImage>(requiredOutputSize));
    wxGraphicsContext* gc = wxGraphicsContext::Create(*compositeImage);

    bool keyFrame = false;

    for ( VideoFramePtr frame : mFrames )
    {
        if (frame->getForceKeyFrame())
        {
            keyFrame = true;
        }
        wxImagePtr image = frame->getImage();
        if (image) // image may be '0' due to clipping/moving
        {
            if (frame->getOpacity()  != 255)
            {
                if (!image->HasAlpha())
                {
                    image->InitAlpha();
                }
                unsigned char* alpha = image->GetAlpha();
                memset(alpha,frame->getOpacity(),image->GetWidth() * image->GetHeight());
            }
            gc->DrawBitmap(gc->GetRenderer()->CreateBitmapFromImage(*image),frame->getPosition().x,frame->getPosition().y,image->GetWidth(),image->GetHeight());
        }
    }

    if (mParameters.getDrawBoundingBox())
    {
        gc->SetPen(wxPen(wxColour(255,255,255), 2));
        gc->SetBrush(wxBrush(wxColour(255,255,255), wxBRUSHSTYLE_TRANSPARENT));
        gc->DrawRectangle( 1, 1, requiredOutputSize.GetWidth() - 1, requiredOutputSize.GetHeight() - 1);
    }

    delete gc;
    VideoFramePtr result = boost::make_shared<VideoFrame>(compositeImage);
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
    os << &obj << '|' << obj.mFrames << '|' << obj.mBoundingBox;
    return os;
}

} //namespace