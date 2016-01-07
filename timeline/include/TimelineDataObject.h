// Copyright 2014-2016 Eric Raijmakers.
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

#include "UtilFrameRate.h"

namespace gui { namespace timeline {

    namespace cmd {
        struct Drop;
        typedef std::vector<Drop> Drops;
    }

class TimelineDataObject
    : public wxTextDataObject
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

    TimelineDataObject(const TimelineDataObject& other) = delete;
    TimelineDataObject& operator=(const TimelineDataObject&) = delete;

    virtual ~TimelineDataObject();

    static const wxString sFormat;

    //////////////////////////////////////////////////////////////////////////
    // FROM wxTextDataObject
    //////////////////////////////////////////////////////////////////////////

    virtual void SetText(const wxString& text) override;

    wxDataFormat GetPreferredFormat(Direction dir=Get) const override;

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

	bool checkIfOkForPasteOrDrop() const;

	/// \return drops to be used in ExecuteDrop for adding the clips to the sequence
	/// \param sequence sequence to be dropped upon (required for looking up tracks)
	/// \param origin origin (in the sequence) of the leftmost added clip
	cmd::Drops getDrops(const model::SequencePtr& sequence, pts sequenceOrigin) const;

    bool storeInClipboard();

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    FrameRate mFrameRate; ///< FrameRate used when the data was put in the clipboard
    int mAudioSampleRate; ///< SampleRate used when the data was put in the clipboard
    cmd::Drops mDropsVideo;
    cmd::Drops mDropsAudio;

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
