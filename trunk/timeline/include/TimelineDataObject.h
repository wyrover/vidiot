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

#ifndef TIMELINE_DATA_OBJECT_H
#define TIMELINE_DATA_OBJECT_H

namespace gui { namespace timeline {

class TimelineDataObject
    :   public wxDataObjectSimple
    ,   public boost::noncopyable
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    TimelineDataObject();

    /// The data in the object is deliberately copied.
    /// Only pointers will be exchanged. By using copies (in the data object)
    /// their lifetime is maintained by the data object.
    /// \param videoTracks list of video clips, divided into tracks
    /// \param audioTracks list of audio clips, divided into tracks
    TimelineDataObject(model::Tracks videoTracks, model::Tracks audioTracks);

    virtual ~TimelineDataObject();

    static const wxString sFormat;

    //////////////////////////////////////////////////////////////////////////
    // FROM wxDataObjectSimple
    //////////////////////////////////////////////////////////////////////////

    virtual bool GetDataHere(void *buf) const override;
    virtual size_t GetDataSize () const override;
    virtual bool SetData(size_t len, const void *buf) override;

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    model::Tracks getVideoTracks() const;
    model::Tracks getAudioTracks() const;

private:

    wxDataFormat mFormat;
    model::Tracks mVideoTracks;
    model::Tracks mAudioTracks;
};

}} // namespace

#endif