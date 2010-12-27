#include "EmptyClip.h"

#include <boost/make_shared.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include "UtilLog.h"
#include "EmptyFile.h"

namespace model {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

EmptyClip::EmptyClip()
    :	Clip()
{
    VAR_DEBUG(this);
}

EmptyClip::EmptyClip(pts length)
    :	Clip(boost::make_shared<EmptyFile>(length))
{
    VAR_DEBUG(this);
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

//////////////////////////////////////////////////////////////////////////
// IAUDIO
//////////////////////////////////////////////////////////////////////////

AudioChunkPtr EmptyClip::getNextAudio(int audioRate, int nAudioChannels)
{
    AudioChunkPtr audioChunk = getDataGenerator<EmptyFile>()->getNextAudio(audioRate, nAudioChannels);
    VAR_AUDIO(audioChunk);
    return audioChunk;
}

//////////////////////////////////////////////////////////////////////////
// IVIDEO
//////////////////////////////////////////////////////////////////////////

VideoFramePtr EmptyClip::getNextVideo(int requestedWidth, int requestedHeight, bool alpha)
{
    VideoFramePtr videoFrame = getDataGenerator<EmptyFile>()->getNextVideo(requestedWidth, requestedHeight, alpha);
    VAR_VIDEO(videoFrame);
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
