#ifndef AUDIO_CHUNK_H
#define AUDIO_CHUNK_H

#include <boost/cstdint.hpp>
#include "UtilFifo.h"

namespace model {

class AudioChunk : boost::noncopyable
{
public:

    static const int sBytesPerSample;

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    /**
     * Initialize, allocate, and fill the data.
     * @param buffer if non-null data from this buffer is copied into the newly allocated space.
     */
    AudioChunk(boost::int16_t* buffer, int nChannels, unsigned int nSamples, double pts);

    /**
    * Initialize but do not allocate yet. Used for empty chunks. Then,
    * allocation only is needed when the data is needed for playback.
    * During 'track combining' empty chunks can be ignored.
    * This avoids needless allocation.
     */
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
    // DATA ACCESS
    //////////////////////////////////////////////////////////////////////////

    /** 
     * Indicates that a number of samples has been consumed by the process
     * that reads chunks.
     * @param samples number of samples that has been read
     */
    void read(unsigned int samples);

    /**
     * Return the data.
     */
    virtual boost::int16_t* getBuffer();

    /**
    * Returns the part of 'getBuffer()' that has not been consumed.
    * Virtual and not const due to reuse in EmptyChunk.
    * @return pointer to first unread sample.
    */
    virtual boost::int16_t* getUnreadSamples();

    unsigned long getUnreadSampleCount() const;

protected:

    boost::int16_t *mBuffer;
    int mNrChannels;
    unsigned int mNrReadSamples;
    unsigned int mNrSamples;
    double mTimeStamp;
};

typedef boost::shared_ptr<AudioChunk> AudioChunkPtr;
typedef Fifo<AudioChunkPtr> FifoAudio;

std::ostream& operator<< (std::ostream& os, const AudioChunk& obj);
std::ostream& operator<< (std::ostream& os, const AudioChunkPtr obj);

} // namespace

#endif // AUDIO_CHUNK_H