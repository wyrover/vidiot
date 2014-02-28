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

#ifndef AUDIO_CHUNK_H
#define AUDIO_CHUNK_H

#include "UtilFifo.h"
#include "UtilInt.h"
#include "UtilRTTI.h"

namespace model {

typedef Fifo<AudioChunkPtr> FifoAudio;
std::ostream& operator<<(std::ostream& os, const AudioChunkPtr obj);

/// Class holds audio samples
///
/// A sample is the data for one speaker.  Its size is typically two bytes.
/// A frame  is the data for all speakers. Its size is 'the number of channels' * 'the size of one sample'
class AudioChunk
    :   public boost::noncopyable
    ,   public IRTTI
{
public:

    static const int sBytesPerSample;

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    /// Initialize, allocate, and (optionally) fill the data.
    /// \param nChannels number of audio channels
    /// \param nSamples number of samples (note: in ALL channels)
    /// \param allocate if true then allocate required buffer
    /// \param zero if true, fill allocated buffer 0
    /// \param buffer if non-null data from this buffer is copied into the newly allocated space
    /// \pre zero   ==> allocate && !buffer
    /// \pre buffer ==> allocate && !zero
    AudioChunk(int nChannels, samplecount nSamples, bool allocate, bool zero, sample* buffer = 0);

    virtual ~AudioChunk();

    //////////////////////////////////////////////////////////////////////////
    // META DATA
    //////////////////////////////////////////////////////////////////////////

    unsigned int getNumberOfChannels() const;

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    /// Try to put the given number of samples into the given data area.
    /// \pre dst must be able to hold requested number of samples
    /// \param dst area to hold the samples
    /// \param requested number of samples to be copied
    /// \return actually stored number of samples (is less than requested if there were no more available samples)
    samplecount extract(uint16_t* dst, samplecount requested);

    /// Indicates that a number of samples has been consumed by the process
    /// that reads chunks.
    /// \param samples number of samples that has been read
    void read(samplecount samples);

    /// Return the data.
    /// Returns the beginning buffer, thus not taking 'read' samples into account.
    virtual sample* getBuffer();

    /// Returns the part of 'getBuffer()' that has not been consumed.
    /// Virtual and not const due to reuse in EmptyChunk.
    /// This method takes 'adjusted length' into account.
    /// \return pointer to first unread sample.
    virtual sample* getUnreadSamples();

    samplecount getUnreadSampleCount() const;

    /// Call this if not all samples in this chunk are 'for use' (typically
    /// required if the extra samples are beyond a clips length.
    void setAdjustedLength(samplecount adjustedLength);

protected:

    sample* mBuffer;                ///< Actual data storage area
    int mNrChannels;                ///< Number of audio channels
    samplecount mNrReadSamples;     ///< Number of samples that has been marked as read using read()
    samplecount mNrSamples;         ///< Total number of samples allocated in memory
    samplecount mNrSkippedSamples;  ///< Set if the length of the chunk is truncated after decoding (for stopping at right pts)

private:

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<<(std::ostream& os, const AudioChunk& obj);
};

} // namespace

#endif
