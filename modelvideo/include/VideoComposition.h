// Copyright 2013-2016 Eric Raijmakers.
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

#include "VideoCompositionParameters.h"

namespace model {

class VideoComposition
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    VideoComposition(const VideoCompositionParameters& parameters);
    VideoComposition(const VideoComposition& other) = delete;
    VideoComposition& operator=(const VideoComposition&) = delete;
    virtual ~VideoComposition();

    //////////////////////////////////////////////////////////////////////////
    // COMPOSITION
    //////////////////////////////////////////////////////////////////////////

    void add(const VideoFramePtr& frame);

    /// Render the composition
    /// \return composition of all input frames.
    /// \note may return '0' to indicate that the composition is completely empty.
    /// \note the pts position value of the returned frame is always 0
    VideoFramePtr generate();

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    VideoCompositionParameters getParameters() const;

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    VideoCompositionParameters mParameters;
    std::vector<VideoFramePtr> mFrames;
    wxSize mBoundingBox;

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<<(std::ostream& os, const VideoComposition& obj);

};

} // namespace
