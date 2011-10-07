#include "EmptyClip.h"

#include <boost/make_shared.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include "UtilLog.h"
#include "EmptyFile.h"
#include "AudioChunk.h"
#include "VideoFrame.h"
#include "Transition.h"

namespace model {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

EmptyClip::EmptyClip()
    :	Clip()
{
    VAR_DEBUG(this);
}

EmptyClip::EmptyClip(pts length, pts extraBegin, pts extraEnd)
    :	Clip(boost::make_shared<EmptyFile>(extraBegin + length + extraEnd))
{
    VAR_DEBUG(this);
    // Ensure that Clip::mOffset and Clip::mLength have the correct values.
    adjustBegin(extraBegin);
    adjustEnd(-extraEnd);
}

EmptyClip::EmptyClip(const EmptyClip& other)
:   Clip(other)
{
    VAR_DEBUG(this);
}

EmptyClip* EmptyClip::clone()
{ 
    return new EmptyClip(static_cast<const EmptyClip&>(*this)); 
}

EmptyClip::~EmptyClip()
{
    VAR_DEBUG(this);
}

// static
EmptyClipPtr EmptyClip::replace( IClipPtr original )
{
    EmptyClipPtr clip;
    if (original->isA<Transition>())
    {
        clip = boost::make_shared<EmptyClip>(original->getLength(), 0, 0);
    }
    else
    {
        clip = boost::make_shared<EmptyClip>(original->getLength(), -1 * original->getMinAdjustBegin(),  original->getMaxAdjustEnd());
        ASSERT_EQUALS(clip->getMaxAdjustBegin(),original->getMaxAdjustBegin());
        ASSERT_EQUALS(clip->getMinAdjustBegin(),original->getMinAdjustBegin());
        ASSERT_EQUALS(clip->getMaxAdjustEnd(),original->getMaxAdjustEnd());
        ASSERT_EQUALS(clip->getMinAdjustEnd(),original->getMinAdjustEnd());
    }
    ASSERT_EQUALS(clip->getLength(),original->getLength());
    return clip;
}

//////////////////////////////////////////////////////////////////////////
// ICONTROL
//////////////////////////////////////////////////////////////////////////

void EmptyClip::clean()
{
    VAR_DEBUG(this);
    Clip::clean();
}

//////////////////////////////////////////////////////////////////////////
// ICLIP
//////////////////////////////////////////////////////////////////////////

void EmptyClip::setLink(IClipPtr link)
{
    // Empty clips may never be linked to anything
    ASSERT(!link)(link);
}

//////////////////////////////////////////////////////////////////////////
// IAUDIO
//////////////////////////////////////////////////////////////////////////

AudioChunkPtr EmptyClip::getNextAudio(int audioRate, int nAudioChannels)
{
    AudioChunkPtr audioChunk = getDataGenerator<EmptyFile>()->getNextAudio(audioRate, nAudioChannels);
    VAR_AUDIO(audioChunk);
    if (audioChunk)
    {
        setGenerationProgress(audioChunk->getPts());
    }
    return audioChunk;
}

//////////////////////////////////////////////////////////////////////////
// IVIDEO
//////////////////////////////////////////////////////////////////////////

VideoFramePtr EmptyClip::getNextVideo(int requestedWidth, int requestedHeight, bool alpha)
{
    VideoFramePtr videoFrame = getDataGenerator<EmptyFile>()->getNextVideo(requestedWidth, requestedHeight, alpha);
    VAR_VIDEO(videoFrame);
    if (videoFrame)
    {
        setGenerationProgress(videoFrame->getPts());
    }
    return videoFrame;
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION 
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void EmptyClip::serialize(Archive & ar, const unsigned int version)
{
    ar & boost::serialization::base_object<Clip>(*this);
    ar & boost::serialization::base_object<IAudio>(*this);
    ar & boost::serialization::base_object<IVideo>(*this);
}
template void EmptyClip::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void EmptyClip::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);

} //namespace
