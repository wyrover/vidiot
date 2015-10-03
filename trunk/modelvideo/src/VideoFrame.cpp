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

#include "VideoFrame.h"

#include "Config.h"
#include "Constants.h"
#include "UtilInitAvcodec.h"
#include "UtilLogBoost.h"
#include "UtilLogWxwidgets.h"
#include "VideoCompositionParameters.h"

namespace model {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

VideoFrame::VideoFrame(const VideoCompositionParameters& parameters)
    : mLayers()
    , mParameters(new VideoCompositionParameters(parameters))
    , mPts(boost::none)
    , mTime(0)
    , mForceKeyFrame(false)
    , mCachedBitmap(boost::none)
{
}

VideoFrame::VideoFrame(const VideoCompositionParameters& parameters, const VideoFrameLayerPtr& layer)
    : mLayers()
    , mParameters(new VideoCompositionParameters(parameters))
    , mPts(boost::none)
    , mTime(0)
    , mForceKeyFrame(false)
    , mCachedBitmap(boost::none)
{
    mLayers.push_back(layer);
}

VideoFrame::VideoFrame(const VideoCompositionParameters& parameters, const VideoFrameLayers& layers)
    : mLayers(layers)
    , mParameters(new VideoCompositionParameters(parameters))
    , mPts(boost::none)
    , mTime(0)
    , mForceKeyFrame(false)
    , mCachedBitmap(boost::none)
{
}

VideoFrame::VideoFrame(const VideoFrame& other)
    : mLayers(make_cloned<VideoFrameLayer>(other.mLayers))
    , mParameters(new VideoCompositionParameters(*other.mParameters))
    , mPts(other.mPts)
    , mTime(other.mTime)
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
    ASSERT(mPts);
    return *mPts;
}

void VideoFrame::setPts(pts position)
{
    mPts.reset(position);
}

rational64 VideoFrame::getTime() const
{
    return mTime;
}

void VideoFrame::setTime(rational64 time)
{
    mTime = time;
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

void VideoFrame::addLayer(const VideoFrameLayerPtr& layer)
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
        wxImagePtr image = getImage();
        if (image != nullptr &&
            image->GetWidth() > 0 &&
            image->GetHeight() > 0)
        {
            mCachedBitmap.reset(boost::make_shared<wxBitmap>(*getImage(), 32));
        }
        else
        {
            mCachedBitmap.reset(wxBitmapPtr());
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
    gc->SetPen(wxPen{wxColour{0,0,0}});
    gc->SetBrush(wxColour{0,0,0});
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

std::ostream& operator<<(std::ostream& os, const VideoFrame& obj)
{
    os  << &obj                     << '|'
        << obj.mPts                 << '|'
        << obj.mTime                << '|'
        << obj.mParameters          << '|'
        << obj.mForceKeyFrame       << '|'
        << obj.mLayers              << '|'
        << typeid(obj).name();
    return os;
}

} // namespace
