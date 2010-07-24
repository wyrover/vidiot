#include "Sequence.h"

#include <boost/make_shared.hpp>
#include <boost/foreach.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/optional.hpp>
#include "GuiMain.h"
#include "VideoTrack.h"
#include "AudioTrack.h"
#include "EmptyFrame.h"
#include "UtilLog.h"
#include "UtilSerializeWxwidgets.h"

namespace model {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

Sequence::Sequence()
    :	IControl()
    ,   mName()
    ,   mVideoTracks()
    ,   mAudioTracks()
{
    VAR_DEBUG(this);
}

Sequence::Sequence(wxString name)
    :	IControl()
    ,   mName(name)
    ,   mVideoTracks()
    ,   mAudioTracks()
{
    VAR_DEBUG(this);
}

Sequence::~Sequence()
{
    VAR_DEBUG(this);
}

//////////////////////////////////////////////////////////////////////////
// SEQUENCE SPECIFIC
//////////////////////////////////////////////////////////////////////////

void Sequence::addVideoTrack(VideoTrackPtr track)
{
    mVideoTracks.push_back(track);
}
void Sequence::addAudioTrack(AudioTrackPtr track)
{
    mAudioTracks.push_back(track);
}
void Sequence::removeVideoTrack(VideoTrackPtr track)
{
    NIY 
}
void Sequence::removeAudioTrack(AudioTrackPtr track)
{
    NIY
}

std::list<VideoTrackPtr> Sequence::getVideoTracks()
{
    return mVideoTracks;
}

std::list<AudioTrackPtr> Sequence::getAudioTracks()
{
    return mAudioTracks;
}

void Sequence::Delete()
{
    // First 'bottom up' reference removal,
    // Second 'top down' reference removal.
    mVideoTracks.clear();
    mAudioTracks.clear();
    mParent.reset();
};

wxString Sequence::getName() const
{ 
    return mName; 
};

void Sequence::setName(wxString name)
{ 
    mName = name;
    gui::wxGetApp().QueueEvent(new model::EventRenameAsset(NodeWithNewName(shared_from_this(),mName)));
}

//////////////////////////////////////////////////////////////////////////
// ICONTROL
//////////////////////////////////////////////////////////////////////////

int64_t Sequence::getNumberOfFrames()
{
    int16_t nFrames = 0;
    BOOST_FOREACH( VideoTrackPtr track, mVideoTracks )
    {
        nFrames = std::max<int64_t>(nFrames, track->getNumberOfFrames());
    }
    BOOST_FOREACH( AudioTrackPtr track, mAudioTracks )
    {
        nFrames = std::max<int64_t>(nFrames, track->getNumberOfFrames());
    }
    return nFrames;
}

void Sequence::moveTo(int64_t position)
{
    BOOST_FOREACH( VideoTrackPtr track, mVideoTracks )
    {
        track->moveTo(position);
    }
    BOOST_FOREACH( AudioTrackPtr track, mAudioTracks )
    {
        track->moveTo(position);
    }
}

//////////////////////////////////////////////////////////////////////////
// IVIDEO
//////////////////////////////////////////////////////////////////////////

VideoFramePtr Sequence::getNextVideo(int requestedWidth, int requestedHeight, bool alpha)
{
    VideoFramePtr videoFrame = (*mVideoTracks.begin())->getNextVideo(requestedWidth, requestedHeight, alpha);
    if (videoFrame && videoFrame->isA<EmptyFrame>())
    {
        VAR_VIDEO(videoFrame);

    }
    VAR_VIDEO(videoFrame);
    return videoFrame;
}

//////////////////////////////////////////////////////////////////////////
// IAUDIO
//////////////////////////////////////////////////////////////////////////

AudioChunkPtr Sequence::getNextAudio(int audioRate, int nAudioChannels)
{
    AudioChunkPtr audioChunk = (*mAudioTracks.begin())->getNextAudio(audioRate, nAudioChannels);
    VAR_AUDIO(audioChunk);
    return audioChunk;
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION 
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void Sequence::serialize(Archive & ar, const unsigned int version)
{
    ar & boost::serialization::base_object<IControl>(*this);
    ar & boost::serialization::base_object<IVideo>(*this);
    ar & boost::serialization::base_object<IAudio>(*this);
    ar & boost::serialization::base_object<AProjectViewNode>(*this);
    ar & mName;
    ar & mVideoTracks;
    ar & mAudioTracks;
}
template void Sequence::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void Sequence::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);

} //namespace
