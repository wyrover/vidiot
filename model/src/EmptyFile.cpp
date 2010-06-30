#include "EmptyFile.h"

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/base_object.hpp>
#include "UtilLog.h"

namespace model {

AudioChunkPtr EmptyFile::EmptyAudioChunk;
VideoFramePtr EmptyFile::EmptyVideoFrame;

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
    if (mAudioPosition > mLength)
    {
        return AudioChunkPtr();
    }
    return EmptyAudioChunk;
}

//////////////////////////////////////////////////////////////////////////
// IVIDEO
//////////////////////////////////////////////////////////////////////////

VideoFramePtr EmptyFile::getNextVideo(int requestedWidth, int requestedHeight, bool alpha)
{
    mVideoPosition++;
    if (mVideoPosition > mLength)
    {
        return VideoFramePtr();
    }
    return EmptyVideoFrame;
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
