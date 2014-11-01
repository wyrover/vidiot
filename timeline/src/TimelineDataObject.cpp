// Copyright 2014 Eric Raijmakers.
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

#include "TimelineDataObject.h"

#include "IClip.h"
#include "UtilLog.h"

namespace gui { namespace timeline {

const wxString TimelineDataObject::sFormat = wxString("application/vidiot/clips");

TimelineDataObject::TimelineDataObject()
: wxDataObjectSimple()
, mFormat(sFormat)
, mVideoTracks()
, mAudioTracks()
{
    SetFormat(mFormat);
}

TimelineDataObject::TimelineDataObject(model::Tracks videoTracks, model::Tracks audioTracks)
: wxDataObjectSimple()
, mFormat(sFormat)
, mVideoTracks(videoTracks)
, mAudioTracks(audioTracks)
{
    SetFormat(mFormat);
}

TimelineDataObject::~TimelineDataObject()
{
}

//////////////////////////////////////////////////////////////////////////
// FROM wxDataObjectSimple
//////////////////////////////////////////////////////////////////////////

bool TimelineDataObject::GetDataHere(void *buf) const
{
    unsigned int* pCounters = static_cast<unsigned int*>(buf);
    *pCounters = mVideoTracks.size();
    ++pCounters;
    *pCounters = mAudioTracks.size();
    ++pCounters;

    model::TrackPtr* tracks = 
        static_cast<model::TrackPtr*>(static_cast<void*>(pCounters));

    for (model::TrackPtr track : mVideoTracks)
    {
        *tracks = track;
        tracks++;
    }
    for (model::TrackPtr track : mAudioTracks)
    {
        *tracks = track;
        tracks++;
    }
    return true;
}

size_t TimelineDataObject::GetDataSize () const
{
    return 
        1 + // First byte indicates number of video tracks
        1 + // Second byte indicates number of audio tracks
        sizeof(model::TrackPtr) * (mVideoTracks.size() + mAudioTracks.size());
}

bool TimelineDataObject::SetData(size_t len, const void *buf)
{
    unsigned int* pCounters = 
        const_cast<unsigned int*>(static_cast<const unsigned int*>(buf));
    unsigned int nVideoTracks = *pCounters;
    ++pCounters;
    unsigned int nAudioTracks = *pCounters;
    ++pCounters;

    model::TrackPtr* tracks =
        static_cast<model::TrackPtr*>(static_cast<void*>(pCounters));

    mVideoTracks.clear();
    for (unsigned int i = 0; i < nVideoTracks; ++i)
    {
        model::TrackPtr track = *tracks;
        mVideoTracks.push_back(track);
        ++tracks;
    }
    mAudioTracks.clear();
    for (unsigned int i = 0; i < nAudioTracks; ++i)
    {
        model::TrackPtr track = *tracks;
        mAudioTracks.push_back(track);
        ++tracks;
    }
    return true;
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

model::Tracks TimelineDataObject::getVideoTracks() const
{
    return mVideoTracks;
}

model::Tracks TimelineDataObject::getAudioTracks() const
{
    return mAudioTracks;
}

}} // namespace