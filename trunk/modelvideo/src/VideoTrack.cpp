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
{
    VAR_DEBUG(this);
}

VideoTrack::VideoTrack(const VideoTrack& other)
:   Track(other)
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
    Track::clean();
}

//////////////////////////////////////////////////////////////////////////
// PLAYBACK
//////////////////////////////////////////////////////////////////////////

VideoFramePtr VideoTrack::getNextVideo(const VideoCompositionParameters& parameters)
{
    VideoFramePtr videoFrame;

    while (!videoFrame && !iterate_atEnd())
    {
        model::IVideoPtr video = boost::dynamic_pointer_cast<IVideo>(iterate_get());
        model::IClipPtr clip = boost::dynamic_pointer_cast<IClip>(iterate_get());
        videoFrame = video->getNextVideo(VideoCompositionParameters(parameters).adjustPts(-clip->getLeftPts()));
        if (!videoFrame)
        {
            iterate_next();
            if (!iterate_atEnd())
            {
                iterate_get()->moveTo(0);
            }
        }
    }
    return videoFrame;
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void VideoTrack::serialize(Archive & ar, const unsigned int version)
{
    try
    {
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Track);
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(IVideo);
    }
    catch (boost::archive::archive_exception& e) { VAR_ERROR(e.what());                         throw; }
    catch (boost::exception &e)                  { VAR_ERROR(boost::diagnostic_information(e)); throw; }
    catch (std::exception& e)                    { VAR_ERROR(e.what());                         throw; }
    catch (...)                                  { LOG_ERROR;                                   throw; }
}
template void VideoTrack::serialize<boost::archive::xml_oarchive>(boost::archive::xml_oarchive& ar, const unsigned int archiveVersion);
template void VideoTrack::serialize<boost::archive::xml_iarchive>(boost::archive::xml_iarchive& ar, const unsigned int archiveVersion);

} //namespace

BOOST_CLASS_EXPORT_IMPLEMENT(model::VideoTrack)