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

#ifndef VIDEO_FRAME_COMPOSITION_H
#define VIDEO_FRAME_COMPOSITION_H

#include "VideoCompositionParameters.h"

namespace model {

class VideoComposition
    : boost::noncopyable
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    VideoComposition(const VideoCompositionParameters& parameters);
    virtual ~VideoComposition();

    //////////////////////////////////////////////////////////////////////////
    // COMPOSITION
    //////////////////////////////////////////////////////////////////////////

    void add(VideoFramePtr frame);

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
    std::list<VideoFramePtr> mFrames;
    wxSize mBoundingBox;

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<<( std::ostream& os, const VideoComposition& obj );

};

} // namespace

#endif // VIDEO_FRAME_COMPOSITION_H