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

#include "UtilFrameRate.h"

namespace gui { namespace timeline {

    namespace command {
        struct Drop;
        typedef std::list<Drop> Drops;
    }

class TimelineDataObject
    : public wxTextDataObject
    , public boost::noncopyable
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    /// Constructor for reading pasted data from the clipboard.
    TimelineDataObject();

    /// Constructor for storing copied data in the clipboard.
    /// This constructor may only be called if there is a project open, since
    /// it requires getting the Properties object.
    /// \param sequence All selected clips in the sequence are made part of the object.
    TimelineDataObject(model::SequencePtr sequence);

    virtual ~TimelineDataObject();

    static const wxString sFormat;

    //////////////////////////////////////////////////////////////////////////
    // FROM wxTextDataObject
    //////////////////////////////////////////////////////////////////////////

    /// Called when data is copied from the clipboard onto this object.
    virtual bool SetData(size_t len, const void *buf) override;

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

	/// \return drops to be used in ExecuteDrop for adding the clips to the sequence
	/// \param sequence sequence to be dropped upon (required for looking up tracks)
	/// \param origin origin (in the sequence) of the leftmost added clip
	command::Drops getDrops(const model::SequencePtr& sequence, pts sequenceOrigin) const;

    bool storeInClipboard();

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    FrameRate mFrameRate; ///< FrameRate used when the data was put in the clipboard
    int mAudioSampleRate; ///< SampleRate used when the data was put in the clipboard
    command::Drops mDropsVideo;
    command::Drops mDropsAudio;

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////


    //////////////////////////////////////////////////////////////////////////
    // SERIALIZATION
    //////////////////////////////////////////////////////////////////////////

    void deserialize(wxString from);
    wxString serialize() const;

    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);
};

}} // namespace

#endif