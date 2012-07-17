#include "Sequence.h"

#include <boost/serialization/list.hpp>
#include <boost/serialization/optional.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include "AudioTrack.h"
#include "EmptyFrame.h"
#include "IClip.h"
#include "NodeEvent.h"
#include "Properties.h"
#include "Render.h"
#include "SequenceEvent.h"
#include "UtilList.h"
#include "UtilLog.h"
#include "UtilLogStl.h"
#include "UtilLogWxwidgets.h"
#include "UtilSerializeWxwidgets.h"
#include "VideoComposition.h"
#include "VideoCompositionParameters.h"
#include "VideoCompositionParameters.h"
#include "VideoTrack.h"
#include "Window.h"

namespace model {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

Sequence::Sequence()
    :   wxEvtHandler()
    ,	IControl()
    ,   IVideo()
    ,   IAudio()
    ,   Node()
    ,   mName()
    ,   mDividerPosition(0)
    ,   mVideoTracks()
    ,   mAudioTracks()
    ,   mVideoTrackMap()
    ,   mAudioTrackMap()
    ,   mPosition(0)
    ,   mRender()
{
    VAR_DEBUG(this);
}

Sequence::Sequence(wxString name)
    :   wxEvtHandler()
    ,	IControl()
    ,   IVideo()
    ,   IAudio()
    ,   Node()
    ,   mName(name)
    ,   mDividerPosition(0)
    ,   mVideoTracks()
    ,   mAudioTracks()
    ,   mVideoTrackMap()
    ,   mAudioTrackMap()
    ,   mPosition(0)
    ,   mRender()
{
    VAR_DEBUG(this);
}

Sequence::Sequence(const Sequence& other)
    :   wxEvtHandler()
    ,	IControl()
    ,   IVideo()
    ,   IAudio()
    ,   Node()
    ,   mName(other.mName)
    ,   mDividerPosition(other.mDividerPosition)
    ,   mVideoTracks(make_cloned<Track>(other.mVideoTracks))
    ,   mAudioTracks(make_cloned<Track>(other.mAudioTracks))
    ,   mVideoTrackMap() // Duplicate administration left empty!
    ,   mAudioTrackMap()  // Duplicate administration left empty!
    ,   mPosition(0)
    ,   mRender()
{
    VAR_DEBUG(this);
}

Sequence* Sequence::clone() const
{
    return new Sequence(static_cast<const Sequence&>(*this));
}

Sequence::~Sequence()
{
    VAR_DEBUG(this);
}

//////////////////////////////////////////////////////////////////////////
// ICONTROL
//////////////////////////////////////////////////////////////////////////

pts Sequence::getLength() const
{
    pts nFrames = 0;
    BOOST_FOREACH( TrackPtr track, mVideoTracks )
    {
        nFrames = std::max<pts>(nFrames, track->getLength());
    }
    BOOST_FOREACH( TrackPtr track, mAudioTracks )
    {
        nFrames = std::max<pts>(nFrames, track->getLength());
    }
    return nFrames;
}

void Sequence::moveTo(pts position)
{
    VAR_DEBUG(position);
    mPosition = position;
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

void Sequence::clean()
{
    VAR_DEBUG(this);
    BOOST_FOREACH( TrackPtr track, getTracks() )
    {
        track->clean();
    }
}

//////////////////////////////////////////////////////////////////////////
// IVIDEO
//////////////////////////////////////////////////////////////////////////

VideoFramePtr Sequence::getNextVideo(const VideoCompositionParameters& parameters)
{
    VideoFramePtr videoFrame = getVideoComposition(parameters)->generate();
    if (videoFrame)
    {
        videoFrame->setPts(mPosition);
        mPosition++;
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
    BOOST_FOREACH( TrackPtr track, tracks )
    {
        track->clean();
    }
    TrackPtr position = UtilList<TrackPtr>(mVideoTracks).removeElements(tracks);
    updateTracks();
    // ProcessEvent is used. Model events must be processed synchronously to avoid inconsistent states in
    // the receivers of these events (typically, the view classes in the timeline). Example: See addVideoTracks.
    ProcessEvent(model::EventRemoveVideoTracks(TrackChange(Tracks(),TrackPtr(),tracks, position)));
}
void Sequence::removeAudioTracks(Tracks tracks)
{
    BOOST_FOREACH( TrackPtr track, tracks )
    {
        track->clean();
    }
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

pixel Sequence::getDividerPosition() const
{
    return mDividerPosition;
}

void Sequence::setDividerPosition(pixel position)
{
    mDividerPosition = position;
}

std::set<model::IClipPtr> Sequence::getSelectedClips()
{
    std::set<model::IClipPtr> selectedclips;
    BOOST_FOREACH( model::TrackPtr track, getTracks() )
    {
        BOOST_FOREACH( model::IClipPtr clip, track->getClips() )
        {
            if (clip->getSelected())
            {
                selectedclips.insert(clip);
            }
        }
    }
    return selectedclips;
}

VideoCompositionPtr Sequence::getVideoComposition(const VideoCompositionParameters& parameters)
{
    VideoCompositionPtr composition(boost::make_shared<VideoComposition>(parameters));
    BOOST_FOREACH( TrackPtr track, mVideoTracks )
    {
        composition->add(boost::dynamic_pointer_cast<IVideo>(track)->getNextVideo(parameters));
    }
    return composition;
}

//////////////////////////////////////////////////////////////////////////
// RENDERING
//////////////////////////////////////////////////////////////////////////

render::RenderPtr Sequence::getRender()
{
    if (!mRender)
    {
        mRender = Properties::get()->getDefaultRender();
        wxFileName name = mRender->getFileName();
        if (!!name.GetPath().IsSameAs(""))
        {
            name.AssignHomeDir();
        }
        name.SetName(getName());
        if (!name.HasExt())
        {
            name.SetExt("avi"); // todo this default must be in config?
        }
        mRender->setFileName(name);
    }
    return mRender;
}

void Sequence::setRender(render::RenderPtr render)
{
    mRender = render;
}

//////////////////////////////////////////////////////////////////////////
// IPROJECTVIEW
//////////////////////////////////////////////////////////////////////////

wxString Sequence::getName() const
{
    return mName; // todo ensure somewhere that no two sequences have the exact same name (anywhere) to avoid problems when rendering?
};

void Sequence::setName(wxString name)
{
    mName = name;
    gui::Window::get().ProcessModelEvent(model::EventRenameNode(NodeWithNewName(shared_from_this(),mName)));
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
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<( std::ostream& os, const Sequence& obj )
{
    os << &obj                           << '|'
        << static_cast<const Node&>(obj) << '|'
        << obj.mDividerPosition          << '|'
        << obj.mVideoTracks              << '|'
        << obj.mAudioTracks              << '|'
        << obj.mPosition                 << '|'
        << obj.mRender;
    return os;
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
    ar & boost::serialization::base_object<Node>(*this);
    ar & mName;
    ar & mDividerPosition;
    ar & mVideoTracks;
    ar & mAudioTracks;
    if (Archive::is_loading::value)
    {
        updateTracks();
    }
    ar & mRender;
}
template void Sequence::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void Sequence::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);

} //namespace