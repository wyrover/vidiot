#ifndef AUDIO_CHUNK_H
#define AUDIO_CHUNK_H

#include "UtilFifo.h"
#include "UtilInt.h"
#include "UtilRTTI.h"

namespace model {

class AudioChunk;
typedef boost::shared_ptr<AudioChunk> AudioChunkPtr;
typedef Fifo<AudioChunkPtr> FifoAudio;
typedef std::list<AudioChunkPtr> AudioChunks;
std::ostream& operator<< (std::ostream& os, const AudioChunkPtr obj);

class AudioChunk
    :   public boost::noncopyable
    ,   public IRTTI
{
public:

    static const int sBytesPerSample;

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    /// Initialize, allocate, and fill the data.
    /// \param buffer if non-null data from this buffer is copied into the newly allocated space.
    /// \param position approximate pts value of this chunk (note: use for debugging only)
    AudioChunk(sample* buffer, int nChannels, samplecount nSamples, pts position);

    /// Initialize but do not allocate yet. Used for empty chunks. Then,
    /// allocation only is needed when the data is needed for playback.
    /// During 'track combining' empty chunks can be ignored.
    /// This avoids needless allocation.
    /// \param position approximate pts value of this chunk (note: use for debugging only)
    AudioChunk(int nChannels, samplecount nSamples, pts position);

    virtual ~AudioChunk();

    //////////////////////////////////////////////////////////////////////////
    // META DATA
    //////////////////////////////////////////////////////////////////////////

    pts getPts() const;
    unsigned int getNumberOfChannels() const;

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

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
    pts mPts;                       ///< Approximate pts value of this chunk (for debugging)

private:

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<< (std::ostream& os, const AudioChunk& obj);
};

} // namespace

#endif // AUDIO_CHUNK_H