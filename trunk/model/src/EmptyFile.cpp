#include "EmptyFile.h"

#include "UtilLog.h"
#include "Convert.h"
#include "Properties.h"
#include "EmptyChunk.h"
#include "EmptyFrame.h"
#include "VideoCompositionParameters.h"

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

EmptyFile::EmptyFile(pts length)
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

EmptyFile* EmptyFile::clone() const
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

pts EmptyFile::getLength() const
{
    return mLength;
}

void EmptyFile::moveTo(pts position)
{
    mAudioPosition = position;
    mVideoPosition = position;
}

wxString EmptyFile::getDescription() const
{
    return wxString();
}

void EmptyFile::clean()
{
    VAR_DEBUG(this);
    mAudioPosition = 0;
    mVideoPosition = 0;
}

//////////////////////////////////////////////////////////////////////////
// IAUDIO
//////////////////////////////////////////////////////////////////////////

AudioChunkPtr EmptyFile::getNextAudio(int audioRate, int nAudioChannels)
{
    ASSERT_LESS_THAN_EQUALS(mAudioPosition,mLength); // Maybe adjustLength() was not directly followed by moveTo()?
    mAudioPosition++;
    if (mAudioPosition >= mLength)
    {
        return AudioChunkPtr();
    }

    // Number of samples for 1 pts
    int nSamples = Convert::ptsToFrames(audioRate,nAudioChannels,1);

    return boost::static_pointer_cast<AudioChunk>(boost::make_shared<EmptyChunk>(nAudioChannels, nSamples, mAudioPosition));
}

//////////////////////////////////////////////////////////////////////////
// IVIDEO
//////////////////////////////////////////////////////////////////////////

VideoFramePtr EmptyFile::getNextVideo(const VideoCompositionParameters& parameters)
{
    ASSERT_LESS_THAN_EQUALS(mVideoPosition,mLength); // Maybe adjustLength() was not directly followed by moveTo()?
    mVideoPosition++;
    if (mVideoPosition >= mLength)
    {
        return VideoFramePtr();
    }

    return boost::static_pointer_cast<VideoFrame>(boost::make_shared<EmptyFrame>(parameters.getBoundingBox(), mVideoPosition));
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void EmptyFile::serialize(Archive & ar, const unsigned int version)
{
    //ar & boost::serialization::base_object<IControl>(*this);//todo remove
    ar & boost::serialization::base_object<IAudio>(*this);
    ar & boost::serialization::base_object<IVideo>(*this);
    ar & mLength;
}
template void EmptyFile::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void EmptyFile::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);

} //namespace