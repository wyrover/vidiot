// Copyright 2013,2014 Eric Raijmakers.
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

#include "UtilLogWxwidgets.h"
#include "Config.h"

namespace model {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

VideoCompositionParameters::VideoCompositionParameters()
    :   mBoundingBox(0,0)
    ,   mDrawBoundingBox(Config::ReadBool(Config::sPathShowBoundingBox))
    ,   mOptimizeForQuality(false)
    ,   mSkip(false)
{
}

VideoCompositionParameters::VideoCompositionParameters(const VideoCompositionParameters& other)
    :   mBoundingBox(other.mBoundingBox)
    ,   mDrawBoundingBox(other.mDrawBoundingBox)
    ,   mOptimizeForQuality(other.mOptimizeForQuality)
    ,   mSkip(other.mSkip)
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
        (mSkip == other.mSkip);
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

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, const VideoCompositionParameters& obj)
{
    os << &obj << '|' << obj.mBoundingBox << '|' << obj.mDrawBoundingBox << '|' << obj.mOptimizeForQuality;
    return os;
}

} //namespace