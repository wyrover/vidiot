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

#include "File.h"
#include "IAudio.h"

struct SwrContext;

namespace model {

class AudioPeaks;

class AudioFile
    :   public File
    ,   public IAudio
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    AudioFile();

    AudioFile(const wxFileName& path);

    virtual AudioFile* clone() const override;

    virtual ~AudioFile();

    //////////////////////////////////////////////////////////////////////////
    // ICONTROL
    //////////////////////////////////////////////////////////////////////////

    virtual void moveTo(pts position) override;
    virtual void clean() override;

    //////////////////////////////////////////////////////////////////////////
    // IAUDIO
    //////////////////////////////////////////////////////////////////////////

    virtual AudioChunkPtr getNextAudio(const AudioCompositionParameters& parameters) override;

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    int getSampleRate();
    int getChannels();
    boost::optional<pts> getNewStartPosition() const;
    AudioPeaks getPeaks(pts offset, pts length);

protected:

    //////////////////////////////////////////////////////////////////////////
    // COPY CONSTRUCTOR
    //////////////////////////////////////////////////////////////////////////

    /// Copy constructor. Use make_cloned for making deep copies of objects.
    /// \see make_cloned
    AudioFile(const AudioFile& other);

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    bool mDecodingAudio;
    bool mNeedsResampling;
    SwrContext* mSoftwareResampleContext;
    int mNrPlanes;

    boost::optional<pts> mNewStartPosition;

    // Do not use the method below for allocating the buffer. That will cause
    // SEGV when compiled with GCC (MingW).
    //int16_t audioDecodeBuffer[AVCODEC_MAX_AUDIO_FRAME_SIZE];
    uint8_t** mAudioDecodeBuffer;

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    void startDecodingAudio(const AudioCompositionParameters& parameters);
    void stopDecodingAudio();

    /// \param pts position (in stream time base units) of a packet returned from ffmpeg
    /// \return number of the first sample in this packet
    samplecount getFirstSample(int64_t pts);

    //////////////////////////////////////////////////////////////////////////
    // FROM FILE
    //////////////////////////////////////////////////////////////////////////

    bool useStream(const AVMediaType& type) const override;

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<<(std::ostream& os, const AudioFile& obj);

    //////////////////////////////////////////////////////////////////////////
    // SERIALIZATION
    //////////////////////////////////////////////////////////////////////////

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);
};

} // namespace

BOOST_CLASS_VERSION(model::AudioFile, 1)
BOOST_CLASS_EXPORT_KEY(model::AudioFile)
