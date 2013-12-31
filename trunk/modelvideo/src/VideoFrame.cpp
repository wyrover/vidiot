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

#include "VideoFrame.h"

#include "Config.h"
#include "Constants.h"
#include "Layout.h"
#include "UtilInitAvcodec.h"
#include "UtilLogWxwidgets.h"
#include "VideoCompositionParameters.h"

namespace model {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

VideoFrame::VideoFrame(const VideoCompositionParameters& parameters)
    : mLayers()
    , mParameters(new VideoCompositionParameters(parameters))
    , mPts(0)
    , mForceKeyFrame(false)
    , mCachedBitmap(boost::none)
{
}

VideoFrame::VideoFrame(const VideoCompositionParameters& parameters, VideoFrameLayerPtr layer)
    : mLayers()
    , mParameters(new VideoCompositionParameters(parameters))
    , mPts(0)
    , mForceKeyFrame(false)
    , mCachedBitmap(boost::none)
{
    mLayers.push_back(layer);
}

VideoFrame::VideoFrame(const VideoCompositionParameters& parameters, VideoFrameLayers layers)
    : mLayers(layers)
    , mParameters(new VideoCompositionParameters(parameters))
    , mPts(0)
    , mForceKeyFrame(false)
    , mCachedBitmap(boost::none)
{
}

VideoFrame::VideoFrame(const VideoFrame& other)
    : mLayers(make_cloned<VideoFrameLayer>(other.mLayers))
    , mParameters(new VideoCompositionParameters(*other.mParameters))
    , mPts(other.mPts)
    , mForceKeyFrame(false)
    , mCachedBitmap(boost::none)
{
}

VideoFrame* VideoFrame::clone() const
{
    return new VideoFrame(static_cast<const VideoFrame&>(*this));
}

void VideoFrame::onCloned()
{
}

VideoFrame::~VideoFrame()
{
}

//////////////////////////////////////////////////////////////////////////
// META DATA
//////////////////////////////////////////////////////////////////////////

pts VideoFrame::getPts() const
{
    return mPts;
}

void VideoFrame::setPts(pts position)
{
    mPts = position;
}

void VideoFrame::setForceKeyFrame(bool force)
{
    mForceKeyFrame = force;
}

bool VideoFrame::getForceKeyFrame() const
{
    return mForceKeyFrame;
}

VideoCompositionParameters VideoFrame::getParameters() const
{
    return *mParameters;
}

VideoFrameLayers VideoFrame::getLayers()
{
    return mLayers;
}

void VideoFrame::addLayer(VideoFrameLayerPtr layer)
{
    mLayers.push_back(layer);
}

wxImagePtr VideoFrame::getImage()
{
    if (mLayers.empty())
    {
        return wxImagePtr();
    }
    wxImagePtr compositeImage(boost::make_shared<wxImage>(mParameters->getBoundingBox()));
    wxGraphicsContext* gc = wxGraphicsContext::Create(*compositeImage);
    if (mParameters->getOptimizeForQuality())
    {
        gc->SetAntialiasMode(wxANTIALIAS_DEFAULT);
        gc->SetCompositionMode(wxCOMPOSITION_OVER);
        gc->SetInterpolationQuality(wxINTERPOLATION_BEST);
    }
    else
    {
        gc->SetAntialiasMode(wxANTIALIAS_NONE);
        gc->SetCompositionMode(wxCOMPOSITION_OVER);
        gc->SetInterpolationQuality(wxINTERPOLATION_NONE);
    }
    VAR_DEBUG(*gc);
    draw(gc);
    delete gc;
    return compositeImage;
}

wxBitmapPtr VideoFrame::getBitmap()
{
    if (!mCachedBitmap)
    {
        if (mLayers.empty())
        {
            mCachedBitmap.reset(wxBitmapPtr());
        }
        else
        {
            mCachedBitmap.reset(boost::make_shared<wxBitmap>(*getImage(), 32));
        }
    }
    return *mCachedBitmap;
}

void VideoFrame::draw(wxGraphicsContext* gc) const
{
    for (VideoFrameLayerPtr layer : mLayers )
    {
        layer->draw(gc, *mParameters);
    }

    wxSize bb(mParameters->getBoundingBox());
    wxRect r(mParameters->getRequiredRectangle());

    // Areas outside the required rectangle are black
    gc->SetPen(*wxBLACK_PEN);
    gc->SetBrush(*wxBLACK_BRUSH);
    gc->DrawRectangle(0,0,r.x,bb.GetHeight());
    gc->DrawRectangle(r.GetRight(), 0, bb.GetWidth() - r.GetRight(), bb.GetHeight());
    gc->DrawRectangle(r.x,0,r.width,r.y);
    gc->DrawRectangle(r.x,r.GetBottom(),r.width,bb.GetHeight() - r.GetBottom());

    if (mParameters->getDrawBoundingBox())
    {
        gc->SetPen(wxPen(wxColour(255,255,255), 2));
        gc->SetBrush(wxBrush(wxColour(255,255,255), wxBRUSHSTYLE_TRANSPARENT));
        gc->DrawRectangle(r.x,r.y,r.width,r.height);
    }
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<< (std::ostream& os, const VideoFrame& obj)
{
    os  << &obj                     << '|'
        << obj.mPts                 << '|'
        << obj.mParameters          << '|'
        << obj.mForceKeyFrame       << '|'
        << obj.mLayers;
    return os;
}

std::ostream& operator<< (std::ostream& os, const VideoFramePtr obj)
{
    if (obj)
    {
        os << *obj;
    }
    else
    {
        os << "0";
    }
    return os;
}

} // namespace