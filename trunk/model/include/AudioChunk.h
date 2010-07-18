#ifndef FIFO_AUDIO_H
#define FIFO_AUDIO_H

#include <boost/cstdint.hpp>
#include "UtilFifo.h"

class AudioChunk : boost::noncopyable
{
public:
    AudioChunk(boost::int16_t* buffer, int nChannels, unsigned int nSamples, double pts);
    virtual ~AudioChunk();

    //////////////////////////////////////////////////////////////////////////
    // META DATA
    //////////////////////////////////////////////////////////////////////////

    double getTimeStamp() const;
    unsigned int getNumberOfChannels() const;

    //////////////////////////////////////////////////////////////////////////
    // UNREAD SAMPLES
    //////////////////////////////////////////////////////////////////////////

    /** 
     * Indicates that a number of samples has been consumed by the process
     * that reads chunks.
     * @param samples number of samples that has been read
     */
    void read(unsigned int samples);

    /**
    * Returns the part of 'getBuffer()' that has not been consumed.
    * @return pointer to first unread sample.
    */
    boost::int16_t* getUnreadSamples() const;

    unsigned long getUnreadSampleCount() const;

    static const int sBytesPerSample;
private:
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

#endif // FIFO_AUDIO_H