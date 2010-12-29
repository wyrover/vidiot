#ifndef AUDIO_CHUNK_H
#define AUDIO_CHUNK_H

#include "UtilFifo.h"
#include "UtilInt.h"

namespace model {

class AudioChunk : boost::noncopyable
{
public:

    static const int sBytesPerSample;

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    /// Initialize, allocate, and fill the data.
    /// @param buffer if non-null data from this buffer is copied into the newly allocated space.
    AudioChunk(boost::int16_t* buffer, int nChannels, unsigned int nSamples, double pts);

    /// Initialize but do not allocate yet. Used for empty chunks. Then,
    /// allocation only is needed when the data is needed for playback.
    /// During 'track combining' empty chunks can be ignored.
    /// This avoids needless allocation.
    AudioChunk(int nChannels, unsigned int nSamples, double pts);

    virtual ~AudioChunk();

    //////////////////////////////////////////////////////////////////////////
    // META DATA
    //////////////////////////////////////////////////////////////////////////

    double getTimeStamp() const;
    unsigned int getNumberOfChannels() const;

    template <typename Derived>
    bool isA()
    {
        return (typeid(Derived) == typeid(*this));
    }

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    /// Indicates that a number of samples has been consumed by the process
    /// that reads chunks.
    /// @param samples number of samples that has been read
    void read(unsigned int samples);

    /// Return the data.
    /// Returns the beginning buffer, thus not taking 'read' samples into account.
    virtual boost::int16_t* getBuffer();

    /// Returns the part of 'getBuffer()' that has not been consumed.
    /// Virtual and not const due to reuse in EmptyChunk.
    /// This method takes 'adjusted length' into account.
    /// @return pointer to first unread sample.
    virtual boost::int16_t* getUnreadSamples();

    unsigned long getUnreadSampleCount() const;

    /// Call this if not all samples in this chunk are 'for use' (typically
    /// required if the extra samples are beyond a clips length.
    void setAdjustedLength(unsigned int adjustedLength);

protected:

    boost::int16_t *mBuffer;            ///< Actual data storage area
    int mNrChannels;                    ///< Number of audio channels
    unsigned int mNrReadSamples;        ///< Number of samples that has been marked as read using read()
    unsigned int mNrSamples;            ///< Total number of samples allocated in memory
    unsigned int mNrSkippedSamples;     ///< Set if the length of the chunk is truncated after decoding (for stopping at right pts)
    double mTimeStamp;

private:

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<< (std::ostream& os, const AudioChunk& obj);
};

typedef boost::shared_ptr<AudioChunk> AudioChunkPtr;
typedef Fifo<AudioChunkPtr> FifoAudio;
typedef std::list<AudioChunkPtr> AudioChunks;
std::ostream& operator<< (std::ostream& os, const AudioChunkPtr obj);

} // namespace

#endif // AUDIO_CHUNK_H