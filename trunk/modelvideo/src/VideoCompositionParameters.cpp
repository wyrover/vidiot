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

#include "VideoCompositionParameters.h"

#include "UtilLogBoost.h"
#include "UtilLogWxwidgets.h"
#include "Config.h"

namespace model {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

VideoCompositionParameters::VideoCompositionParameters()
    : mBoundingBox(0, 0)
    , mDrawBoundingBox(false)
    , mOptimizeForQuality(false)
    , mSkip(false)
    , mPts(boost::none)
{
}

VideoCompositionParameters::VideoCompositionParameters(const VideoCompositionParameters& other)
    : mBoundingBox(other.mBoundingBox)
    , mDrawBoundingBox(other.mDrawBoundingBox)
    , mOptimizeForQuality(other.mOptimizeForQuality)
    , mSkip(other.mSkip)
    , mPts(other.mPts)
{
}

//////////////////////////////////////////////////////////////////////////
// OPERATORS
//////////////////////////////////////////////////////////////////////////

bool VideoCompositionParameters::operator==( const VideoCompositionParameters& other )
{
    return
        (mBoundingBox == other.mBoundingBox) &&
        (mDrawBoundingBox == other.mDrawBoundingBox) &&
        (mOptimizeForQuality == other.mOptimizeForQuality) &&
        (mSkip == other.mSkip) &&
        (mPts == other.mPts);
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

VideoCompositionParameters& VideoCompositionParameters::setBoundingBox(const wxSize& boundingBox)
{
    mBoundingBox = boundingBox;
    ASSERT_MORE_THAN_ZERO(mBoundingBox.GetWidth());
    ASSERT_MORE_THAN_ZERO(mBoundingBox.GetHeight());
    ASSERT_MORE_THAN_EQUALS_ZERO(mBoundingBox.GetX());
    ASSERT_MORE_THAN_EQUALS_ZERO(mBoundingBox.GetY());
    return *this;
}

wxSize VideoCompositionParameters::getBoundingBox() const
{
    return mBoundingBox;
}

VideoCompositionParameters& VideoCompositionParameters::setDrawBoundingBox(bool draw)
{
    mDrawBoundingBox = draw;
    return *this;
}

bool VideoCompositionParameters::getDrawBoundingBox() const
{
    return mDrawBoundingBox;
}

VideoCompositionParameters& VideoCompositionParameters::setOptimizeForQuality()
{
    mOptimizeForQuality = true;
    return *this;
}

bool VideoCompositionParameters::getOptimizeForQuality() const
{
    return mOptimizeForQuality;
}

VideoCompositionParameters& VideoCompositionParameters::setSkip(bool skip)
{
    mSkip = skip;
    return *this;
}

bool VideoCompositionParameters::getSkip() const
{
    return mSkip;
}

wxRect VideoCompositionParameters::getRequiredRectangle() const
{
    boost::rational<int> scaleToBoundingBox(0);
    wxSize requiredOutputSize = Convert::sizeInBoundingBox(Properties::get().getVideoSize(), mBoundingBox, scaleToBoundingBox);
    wxPoint requiredOutputOffset((mBoundingBox.x - requiredOutputSize.x ) / 2, (mBoundingBox.y - requiredOutputSize.y ) / 2);
    return wxRect(requiredOutputOffset, requiredOutputSize);
}

VideoCompositionParameters& VideoCompositionParameters::setPts(pts position)
{
    mPts.reset(position);
    return *this;
}

VideoCompositionParameters& VideoCompositionParameters::adjustPts(pts adjustment)
{
    if (mPts)
    {
        *mPts += adjustment;
    }
    return *this;
}

bool VideoCompositionParameters::hasPts() const
{
    return static_cast<bool>(mPts);
}

pts VideoCompositionParameters::getPts() const
{
    ASSERT(mPts);
    return *mPts;
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, const VideoCompositionParameters& obj)
{
    os  << &obj << '|' 
        << obj.mBoundingBox << '|' 
        << obj.mDrawBoundingBox << '|' 
        << obj.mOptimizeForQuality << '|' 
        << obj.mSkip << '|' 
        << obj.mPts;
    return os;
}

} //namespace