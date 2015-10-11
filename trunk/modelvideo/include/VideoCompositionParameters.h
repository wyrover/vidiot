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

#pragma once

#include "IVideo.h"

namespace model {

class VideoCompositionParameters
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    VideoCompositionParameters();
    VideoCompositionParameters(const VideoCompositionParameters& other);

    //////////////////////////////////////////////////////////////////////////
    // OPERATORS
    //////////////////////////////////////////////////////////////////////////

    bool operator==( const VideoCompositionParameters& other );

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    VideoCompositionParameters& setBoundingBox(const wxSize& boundingBox);
    wxSize getBoundingBox() const;

    VideoCompositionParameters& setDrawBoundingBox(bool draw);
    bool getDrawBoundingBox() const;

    VideoCompositionParameters& setOptimizeForQuality();
    bool getOptimizeForQuality() const;

    VideoCompositionParameters& setSkip(bool skip);
    bool getSkip() const;

    VideoCompositionParameters& setPts(pts position);
    VideoCompositionParameters& adjustPts(pts adjustment); ///< \note Does nothing if there is no pts stored
    bool hasPts() const;
    pts getPts() const;

    wxRect getRequiredRectangle() const;

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    wxSize mBoundingBox;
    bool mDrawBoundingBox;
    bool mOptimizeForQuality;
    bool mSkip;
    boost::optional<pts> mPts;

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<<(std::ostream& os, const VideoCompositionParameters& obj);

};

} // namespace
