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
#include "UtilLogStl.h"
#include "UtilSerializeWxwidgets.h"
#include "UtilList.h"

namespace model {

DEFINE_EVENT(EVENT_ADD_VIDEO_TRACK,      EventAddVideoTracks,      TrackChange);
DEFINE_EVENT(EVENT_REMOVE_VIDEO_TRACK,   EventRemoveVideoTracks,   TrackChange);
DEFINE_EVENT(EVENT_ADD_AUDIO_TRACK,      EventAddAudioTracks,      TrackChange);
DEFINE_EVENT(EVENT_REMOVE_AUDIO_TRACK,   EventRemoveAudioTracks,   TrackChange);

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

Sequence::Sequence()
    :	IControl()
    ,   wxEvtHandler()
    ,   mName()
    ,   mVideoTracks()
    ,   mAudioTracks()
    ,   mVideoTrackMap()
    ,   mAudioTrackMap()
{
    VAR_DEBUG(this);
}

Sequence::Sequence(wxString name)
    :	IControl()
    ,   mName(name)
    ,   mVideoTracks()
    ,   mAudioTracks()
    ,   mVideoTrackMap()
    ,   mAudioTrackMap()
{
    VAR_DEBUG(this);
}

Sequence::Sequence(const Sequence& other)
:	IControl()
,   mName(other.mName)
,   mVideoTracks(other.mVideoTracks)
,   mAudioTracks(other.mAudioTracks)
,   mVideoTrackMap(other.mVideoTrackMap)
,   mAudioTrackMap(other.mVideoTrackMap)
{
    VAR_DEBUG(this);
}

Sequence* Sequence::clone()
{ 
    return new Sequence(static_cast<const Sequence&>(*this)); 
}

Sequence::~Sequence()
{
    VAR_DEBUG(this);
    /// @todo remove all tracks recursively (note: the tracks do not keep
    /// references to the sequence, hence the sequence can simply go
    /// out-of-scope (smartptr) and then this destructor is called. The
    /// tracks however are kept alive via their child clips, thus they
    /// need explicit cleanup.
}

//////////////////////////////////////////////////////////////////////////
// SEQUENCE SPECIFIC
//////////////////////////////////////////////////////////////////////////

void Sequence::addVideoTracks(Tracks tracks, TrackPtr position)
{
    UtilList<TrackPtr>(mVideoTracks).addElements(tracks,position);
    updateTracks();
    // ProcessEvent is used. Model events must be processed synchronously to avoid inconsistent states in
    // the receivers of these events (typically, the view classes in the timeline).
    // Example: Create sequence from autofolder.
    // 1. Sequence is created
    // 2. Tracks are added (and if queue events were used these are queued here)
    // 3. Sequence is opened and initial tracks are 'added' as views
    // 4. At some later point, the queued events result in 'double' views for the added tracks.
    ProcessEvent(model::EventAddVideoTracks(TrackChange(tracks, position)));
}

void Sequence::addAudioTracks(Tracks tracks, TrackPtr position)
{
    UtilList<TrackPtr>(mAudioTracks).addElements(tracks,position);
    updateTracks();
    // ProcessEvent is used. Model events must be processed synchronously to avoid inconsistent states in
    // the receivers of these events (typically, the view classes in the timeline). Example: See addVideoTracks.
    ProcessEvent(model::EventAddAudioTracks(TrackChange(tracks, position)));
}
void Sequence::removeVideoTracks(Tracks tracks)
{
    TrackPtr position = UtilList<TrackPtr>(mVideoTracks).removeElements(tracks);
    updateTracks();
    // ProcessEvent is used. Model events must be processed synchronously to avoid inconsistent states in
    // the receivers of these events (typically, the view classes in the timeline). Example: See addVideoTracks.
    ProcessEvent(model::EventRemoveVideoTracks(TrackChange(Tracks(),TrackPtr(),tracks, position)));
}
void Sequence::removeAudioTracks(Tracks tracks)
{
    TrackPtr position = UtilList<TrackPtr>(mAudioTracks).removeElements(tracks);
    updateTracks();
    // ProcessEvent is used. Model events must be processed synchronously to avoid inconsistent states in
    // the receivers of these events (typically, the view classes in the timeline). Example: See addVideoTracks.
    ProcessEvent(model::EventRemoveAudioTracks(TrackChange(Tracks(),TrackPtr(),tracks, position)));
}

Tracks Sequence::getVideoTracks()
{
    return mVideoTracks;
}

Tracks Sequence::getAudioTracks()
{
    return mAudioTracks;
}

Tracks Sequence::getTracks()
{
    Tracks tracks;
    BOOST_FOREACH( TrackPtr track, mVideoTracks )
    {
        tracks.push_back(track);
    }
    BOOST_FOREACH( TrackPtr track, mAudioTracks )
    {
        tracks.push_back(track);
    }
    return tracks;
}

TrackPtr Sequence::getVideoTrack(int index)
{
    return mVideoTrackMap[index];
}

TrackPtr Sequence::getAudioTrack(int index)
{
    return mAudioTrackMap[index];
}

//////////////////////////////////////////////////////////////////////////
// IPROJECTVIEW
//////////////////////////////////////////////////////////////////////////

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
    BOOST_FOREACH( TrackPtr track, mVideoTracks )
    {
        nFrames = std::max<int64_t>(nFrames, track->getNumberOfFrames());
    }
    BOOST_FOREACH( TrackPtr track, mAudioTracks )
    {
        nFrames = std::max<int64_t>(nFrames, track->getNumberOfFrames());
    }
    return nFrames;
}

void Sequence::moveTo(int64_t position)
{
    BOOST_FOREACH( TrackPtr track, mVideoTracks )
    {
        track->moveTo(position);
    }
    BOOST_FOREACH( TrackPtr track, mAudioTracks )
    {
        track->moveTo(position);
    }
}

wxString Sequence::getDescription() const
{
    return getName();
}

//////////////////////////////////////////////////////////////////////////
// IVIDEO
//////////////////////////////////////////////////////////////////////////

VideoFramePtr Sequence::getNextVideo(int requestedWidth, int requestedHeight, bool alpha)
{
    VideoFrames frames;
    BOOST_FOREACH( TrackPtr track, mVideoTracks )
    {
        VideoFramePtr videoFrame =  boost::dynamic_pointer_cast<IVideo>(track)->getNextVideo(requestedWidth, requestedHeight, alpha);
        frames.push_back(videoFrame);
    }
    VideoFramePtr videoFrame; // Default: Null ptr (at end)
    BOOST_REVERSE_FOREACH( VideoFramePtr frame, frames )
    {
        if (frame)
        {
            if (frame->isA<EmptyFrame>())
            {
                // At least send the EmptyFrame (instead of a null ptr which indiates 'end').
                // Do not exit the loop: one of the lower tracks may have a bitmap.
                videoFrame = frame;
            }
            else
            {
                // From the top track, the first found frame is returned. 
                videoFrame = frame;
                break;
            }
        }
    }
    VAR_VIDEO(videoFrame);
    return videoFrame;
}

//////////////////////////////////////////////////////////////////////////
// IAUDIO
//////////////////////////////////////////////////////////////////////////

AudioChunkPtr Sequence::getNextAudio(int audioRate, int nAudioChannels)
{
    AudioChunkPtr audioChunk = boost::dynamic_pointer_cast<IAudio>(*mAudioTracks.begin())->getNextAudio(audioRate, nAudioChannels);
    VAR_AUDIO(audioChunk);
    return audioChunk;
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void Sequence::updateTracks()
{
    int index = 0;
    mVideoTrackMap.clear();
    BOOST_FOREACH( TrackPtr track, mVideoTracks )
    {
        track->setIndex(index);
        mVideoTrackMap[index] = track;
        ++index;
    }
    index = 0;
    mAudioTrackMap.clear();
    BOOST_FOREACH( TrackPtr track, mAudioTracks )
    {
        track->setIndex(index);
        mAudioTrackMap[index] = track;
        ++index;
    }
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
    if (Archive::is_loading::value)
    {
        updateTracks();
    }
}
template void Sequence::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void Sequence::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);

} //namespace
