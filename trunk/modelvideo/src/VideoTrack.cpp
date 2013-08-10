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

#include "VideoTrack.h"

#include "UtilLog.h"
#include "VideoClip.h"
#include "VideoFrame.h"

namespace model {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

VideoTrack::VideoTrack()
:	Track()
,   mPts(0)
{
    VAR_DEBUG(this);
}

VideoTrack::VideoTrack(const VideoTrack& other)
:   Track(other)
,   mPts(0)
{
    VAR_DEBUG(this);
}

VideoTrack* VideoTrack::clone() const
{
    return new VideoTrack(static_cast<const VideoTrack&>(*this));
}

VideoTrack::~VideoTrack()
{
    VAR_DEBUG(this);
}

//////////////////////////////////////////////////////////////////////////
// ICONTROL
//////////////////////////////////////////////////////////////////////////

void VideoTrack::clean()
{
    VAR_DEBUG(this);
    mPts = 0;
    Track::clean();
}

//////////////////////////////////////////////////////////////////////////
// PLAYBACK
//////////////////////////////////////////////////////////////////////////

void VideoTrack::moveTo(pts position)
{
    Track::moveTo(position);
    mPts = position;
}

VideoFramePtr VideoTrack::getNextVideo(const VideoCompositionParameters& parameters)
{
    VideoFramePtr videoFrame;

    while (!videoFrame && !iterate_atEnd())
    {
        videoFrame = boost::dynamic_pointer_cast<IVideo>(iterate_get())->getNextVideo(parameters);
        if (!videoFrame)
        {
            iterate_next();
            if (!iterate_atEnd())
            {
                iterate_get()->moveTo(0);
            }
        }
    }
    if (videoFrame)
    {
        videoFrame->setPts(mPts++);
    }
    VAR_VIDEO(videoFrame);
    return videoFrame;
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void VideoTrack::serialize(Archive & ar, const unsigned int version)
{
    ar & boost::serialization::base_object<Track>(*this);
    ar & boost::serialization::base_object<IVideo>(*this);
}
template void VideoTrack::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void VideoTrack::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);

} //namespace