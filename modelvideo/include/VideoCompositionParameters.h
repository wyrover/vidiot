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

#ifndef VIDEO_FRAME_COMPOSITION_PARAMETERS_H
#define VIDEO_FRAME_COMPOSITION_PARAMETERS_H

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

    VideoCompositionParameters& setBoundingBox(wxSize boundingBox);
    wxSize getBoundingBox() const;

    VideoCompositionParameters& setDrawBoundingBox(bool draw);
    bool getDrawBoundingBox() const;

    VideoCompositionParameters& setOptimizeForQuality();
    bool getOptimizeForQuality() const;

    VideoCompositionParameters& setSkip(bool skip);
    bool getSkip() const;

    wxRect getRequiredRectangle() const;

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    wxSize mBoundingBox;
    bool mDrawBoundingBox;
    bool mOptimizeForQuality;
    bool mSkip;

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<<( std::ostream& os, const VideoCompositionParameters& obj );

};

} // namespace

#endif // VIDEO_FRAME_COMPOSITION_PARAMETERS_H