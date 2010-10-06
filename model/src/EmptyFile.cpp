#include "EmptyFile.h"

#include <boost/make_shared.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/base_object.hpp>
#include "UtilLog.h"
#include "Project.h"
#include "Properties.h"
#include "EmptyChunk.h"
#include "EmptyFrame.h"

namespace model {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

EmptyFile::EmptyFile()
:   mLength(0)
,   mAudioPosition(0)
,   mVideoPosition(0)
{ 
    VAR_DEBUG(this);
}

EmptyFile::EmptyFile(boost::int64_t length)
:   mLength(length)
,   mAudioPosition(0)
,   mVideoPosition(0)
{ 
    VAR_DEBUG(this)(mLength);
}

EmptyFile::EmptyFile(const EmptyFile& other)
:   mLength(other.mLength)
,   mAudioPosition(0)
,   mVideoPosition(0)
{ 
    VAR_DEBUG(this)(mLength);
}

EmptyFile* EmptyFile::clone()
{ 
    return new EmptyFile(static_cast<const EmptyFile&>(*this)); 
}

EmptyFile::~EmptyFile()
{
    VAR_DEBUG(this);
}

//////////////////////////////////////////////////////////////////////////
// ICONTROL
//////////////////////////////////////////////////////////////////////////

boost::int64_t EmptyFile::getNumberOfFrames()
{
    return mLength;
}

void EmptyFile::moveTo(boost::int64_t position)
{
    mAudioPosition = position;
    mVideoPosition = position;
}

//////////////////////////////////////////////////////////////////////////
// IAUDIO
//////////////////////////////////////////////////////////////////////////

AudioChunkPtr EmptyFile::getNextAudio(int audioRate, int nAudioChannels)
{
    mAudioPosition++;
    if (mAudioPosition >= mLength)
    {
        return AudioChunkPtr();
    }
    /** todo correct pts (third parameter) */
    /** todo more precise n samples */

    // Number of samples for 1 video frame
    int nSamples = nAudioChannels * static_cast<int>(floor(boost::rational_cast<double>(audioRate * Project::current()->getProperties()->getFrameRate())));

    return boost::static_pointer_cast<AudioChunk>(boost::make_shared<EmptyChunk>(nAudioChannels, nSamples, mAudioPosition));
} 

//////////////////////////////////////////////////////////////////////////
// IVIDEO
//////////////////////////////////////////////////////////////////////////

VideoFramePtr EmptyFile::getNextVideo(int requestedWidth, int requestedHeight, bool alpha)
{
    mVideoPosition++;
    if (mVideoPosition >= mLength)
    {
        return VideoFramePtr();
    }

    PixelFormat format = PIX_FMT_RGBA;
    if (!alpha)
    {
        format = PIX_FMT_RGB24;
    }

    /** todo timebase */
    AVRational timebase;
    timebase.num = 0;
    timebase.den = 0;
    return boost::static_pointer_cast<VideoFrame>(boost::make_shared<EmptyFrame>(format, requestedWidth, requestedHeight, mVideoPosition, timebase));
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION 
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void EmptyFile::serialize(Archive & ar, const unsigned int version)
{
    ar & boost::serialization::base_object<IControl>(*this);
    ar & boost::serialization::base_object<IAudio>(*this);
    ar & boost::serialization::base_object<IVideo>(*this);
    ar & mLength;
}
template void EmptyFile::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void EmptyFile::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);

} //namespace