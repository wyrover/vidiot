// Copyright 2013,2014 Eric Raijmakers.
//
// This file is part of Vidiot.
//
// Vidiot is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Vidiot is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Vidiot. If not, see <http://www.gnu.org/licenses/>.

#include "Sequence.h"

#include "AudioComposition.h"
#include "AudioCompositionParameters.h"
#include "AudioTrack.h"
#include "EmptyFrame.h"
#include "IClip.h"
#include "Logging.h"
#include "ModelEvent.h"
#include "NodeEvent.h"
#include "ProjectModification.h"
#include "Properties.h"
#include "Render.h"
#include "SequenceEvent.h"
#include "UtilClone.h"
#include "UtilList.h"
#include "UtilLog.h"
#include "UtilLogStl.h"
#include "UtilLogWxwidgets.h"
#include "UtilSerializeWxwidgets.h"
#include "UtilSet.h"
#include "VideoComposition.h"
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
    ,   mVideoPosition(0)
    ,   mRender()
{
    VAR_DEBUG(this);
    // Serialization will fill in the members
    // and call updateTracks()
}

Sequence::Sequence(const wxString& name)
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
    ,   mVideoPosition(0)
    ,   mRender()
{
    VAR_DEBUG(this);
    mVideoTracks.push_back(boost::make_shared<VideoTrack>());
    mAudioTracks.push_back(boost::make_shared<AudioTrack>());
    updateTracks();
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
    ,   mVideoTrackMap() // Duplicate administration left empty! (This constructor should only be used for cloning directly before rendering)
    ,   mAudioTrackMap()  // Duplicate administration left empty! (...and for rendering the duplicate administration is not required)
    ,   mVideoPosition(0)
    ,   mRender(make_cloned<render::Render>(other.mRender))
{
    VAR_DEBUG(this);
    updateTracks();
}

Sequence* Sequence::clone() const
{
    return new Sequence(static_cast<const Sequence&>(*this));
}

void Sequence::onCloned()
{
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
    for ( TrackPtr track : mVideoTracks )
    {
        nFrames = std::max<pts>(nFrames, track->getLength());
    }
    for ( TrackPtr track : mAudioTracks )
    {
        nFrames = std::max<pts>(nFrames, track->getLength());
    }
    return nFrames;
}

void Sequence::moveTo(const pts& position)
{
    VAR_DEBUG(position);
    mVideoPosition = position;
    for ( TrackPtr track : mVideoTracks )
    {
        track->moveTo(position);
    }
    for ( TrackPtr track : mAudioTracks )
    {
        track->moveTo(position);
    }
    mCache.cachedAudio.clear();
}

wxString Sequence::getDescription() const
{
    return getName();
}

void Sequence::clean()
{
    VAR_DEBUG(this);
    for ( TrackPtr track : getTracks() )
    {
        track->clean();
    }
    mCache.cachedAudio.clear();
}

//////////////////////////////////////////////////////////////////////////
// IVIDEO
//////////////////////////////////////////////////////////////////////////

VideoFramePtr Sequence::getNextVideo(const VideoCompositionParameters& parameters)
{
    VideoCompositionPtr composition = getVideoComposition(parameters);
    VideoFramePtr videoFrame = composition->generate();
    if (videoFrame)
    {
        videoFrame->setPts(mVideoPosition);
        mVideoPosition++;
    }
    VAR_VIDEO(videoFrame);
    return videoFrame;
}

//////////////////////////////////////////////////////////////////////////
// IAUDIO
//////////////////////////////////////////////////////////////////////////

AudioChunkPtr Sequence::getNextAudio(const AudioCompositionParameters& parameters)
{
    AudioCompositionPtr composition = getAudioComposition(parameters);
    AudioChunkPtr audioChunk = composition->generate();
    if (composition->wasInputChunkReturnedAsOutput())
    {
        // The chunk was forwarded immediately to the 'consumer'.
        // (Performance optimization for the '1 track only' case).
        // However, no changes in number of read samples were made yet
        // (since that's to be done by the consumer).
        // Therefore, ensure that a 'getNextAudio' call is done.
        mCache.cachedAudio.clear();
    }
    VAR_VIDEO(audioChunk);
    return audioChunk;
}

//////////////////////////////////////////////////////////////////////////
// SEQUENCE SPECIFIC
//////////////////////////////////////////////////////////////////////////

void Sequence::addVideoTracks(const Tracks& tracks, const TrackPtr& position)
{
    for ( model::TrackPtr track : tracks )
    {
         track->Bind(model::EVENT_LENGTH_CHANGED, &Sequence::onTrackLengthChanged, this);
    }

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

    // This may NOT be called before the add/remove event is sent: updateLength() may cause view updates,
    // which cause accesses to the model. By that time, all views must know the proper list of tracks.
    updateLength();
}

void Sequence::addAudioTracks(const Tracks& tracks, const TrackPtr& position)
{
    for ( model::TrackPtr track : tracks )
    {
         track->Bind(model::EVENT_LENGTH_CHANGED, &Sequence::onTrackLengthChanged, this);
    }

    UtilList<TrackPtr>(mAudioTracks).addElements(tracks,position);

    updateTracks();

    // ProcessEvent is used. Model events must be processed synchronously to avoid inconsistent states in
    // the receivers of these events (typically, the view classes in the timeline). Example: See addVideoTracks.
    ProcessEvent(model::EventAddAudioTracks(TrackChange(tracks, position)));

    // This may NOT be called before the add/remove event is sent: updateLength() may cause view updates,
    // which cause accesses to the model. By that time, all views must know the proper list of tracks.
    updateLength();
}

void Sequence::removeVideoTracks(const Tracks& tracks)
{
    for ( TrackPtr track : tracks )
    {
        track->clean();
        track->Unbind(model::EVENT_LENGTH_CHANGED, &Sequence::onTrackLengthChanged, this);
    }
    TrackPtr position = UtilList<TrackPtr>(mVideoTracks).removeElements(tracks);

    updateTracks();
    // ProcessEvent is used. Model events must be processed synchronously to avoid inconsistent states in
    // the receivers of these events (typically, the view classes in the timeline). Example: See addVideoTracks.
    ProcessEvent(model::EventRemoveVideoTracks(TrackChange(Tracks(),TrackPtr(),tracks, position)));

    // This may NOT be called before the add/remove event is sent: updateLength() may cause view updates,
    // which cause accesses to the model. By that time, all views must know the proper list of tracks.
    updateLength();
}

void Sequence::removeAudioTracks(const Tracks& tracks)
{
    for ( TrackPtr track : tracks )
    {
        track->clean();
        track->Unbind(model::EVENT_LENGTH_CHANGED, &Sequence::onTrackLengthChanged, this);
    }
    TrackPtr position = UtilList<TrackPtr>(mAudioTracks).removeElements(tracks);

    updateTracks();
    // ProcessEvent is used. Model events must be processed synchronously to avoid inconsistent states in
    // the receivers of these events (typically, the view classes in the timeline). Example: See addVideoTracks.
    ProcessEvent(model::EventRemoveAudioTracks(TrackChange(Tracks(),TrackPtr(),tracks, position)));

    // This may NOT be called before the add/remove event is sent: updateLength() may cause view updates,
    // which cause accesses to the model. By that time, all views must know the proper list of tracks.
    updateLength();
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
    for ( TrackPtr track : mVideoTracks )
    {
        tracks.push_back(track);
    }
    for ( TrackPtr track : mAudioTracks )
    {
        tracks.push_back(track);
    }
    return tracks;
}

TrackPtr Sequence::getVideoTrack(const int& index)
{
    return mVideoTrackMap[index];
}

TrackPtr Sequence::getAudioTrack(const int& index)
{
    return mAudioTrackMap[index];
}

pixel Sequence::getDividerPosition() const
{
    return mDividerPosition;
}

void Sequence::setDividerPosition(const pixel& position)
{
    if (mDividerPosition != position)
    {
        mDividerPosition = position;
        model::ProjectModification::trigger();
    }
}

std::set<model::IClipPtr> Sequence::getSelectedClips()
{
    std::set<model::IClipPtr> selectedclips;
    for ( model::TrackPtr track : getTracks() )
    {
        for ( model::IClipPtr clip : track->getClips() )
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
    for ( TrackPtr track : mVideoTracks )
    {
        composition->add(boost::dynamic_pointer_cast<IVideo>(track)->getNextVideo(parameters));
    }
    return composition;
}

AudioCompositionPtr Sequence::getAudioComposition(const AudioCompositionParameters& parameters)
{
    AudioCompositionPtr composition(boost::make_shared<AudioComposition>(parameters));
    for ( TrackPtr track : mAudioTracks )
    {
        std::map< TrackPtr, AudioChunkPtr >::iterator it = mCache.cachedAudio.find(track);
        if (it != mCache.cachedAudio.end())
        {
            if (it->second)
            {
                if (it->second->getUnreadSampleCount() == 0)
                {
                    mCache.cachedAudio.erase(it); // chunk used completely; get next chunk
                }
                // else: cached chunk not completely used yet; use current chunk
            }
            // else: end of track reached; keep using this chunk
        }
        // else: no cached chunk yet; get first chunk

        if (mCache.cachedAudio.find(track) == mCache.cachedAudio.end())
        {
            mCache.cachedAudio[ track ] = boost::dynamic_pointer_cast<IAudio>(track)->getNextAudio(parameters);
        }

        if (mCache.cachedAudio[ track ])
        {
            // Only add non 0 ptrs (if at end of track, then skip)
            composition->add(mCache.cachedAudio[ track ]);
        }

    }
    return composition;
}

std::set<pts> Sequence::getCuts(const std::set<IClipPtr>& exclude)
{
    // PERF: cache this?
    std::set<pts> result;
    for ( TrackPtr track : getTracks() )
    {
        UtilSet<pts>(result).addElements(track->getCuts(exclude));
    }
    VAR_DEBUG(result);
    return result;
}

bool Sequence::isEmptyAt(const pts& position ) const
{
    for ( TrackPtr track : mVideoTracks )
    {
        if (!track->isEmptyAt(position)) { return false; }
    }
    for ( TrackPtr track : mAudioTracks )
    {
        if (!track->isEmptyAt(position)) { return false; }
    }
    return true;
}

void Sequence::onTrackLengthChanged(EventLengthChanged& event)
{
    updateLength();
    event.Skip();
}

//////////////////////////////////////////////////////////////////////////
// RENDERING
//////////////////////////////////////////////////////////////////////////

render::RenderPtr Sequence::getRender()
{
    if (!mRender)
    {
        mRender = Properties::get().getDefaultRender();
        wxFileName name = mRender->getFileName();
        if (!!name.GetPath().IsSameAs(""))
        {
            name.AssignHomeDir();
        }
        name.SetName(getName());
        if (!name.HasExt())
        {
            name.SetExt("avi"); // Default is avi. User can set a new default extension via the RenderSettingsDialog
        }
        mRender->setFileName(name);
    }
    return mRender;
}

void Sequence::setRender(const render::RenderPtr& render)
{
    mRender = render;
}

//////////////////////////////////////////////////////////////////////////
// INODE
//////////////////////////////////////////////////////////////////////////

wxString Sequence::getName() const
{
    return mName;
};

void Sequence::setName(const wxString& name)
{
    mName = name;
    gui::Window::get().ProcessModelEvent(model::EventRenameNode(NodeWithNewName(self(),mName)));
}

void Sequence::check()
{
    // Nothing is checked: If files are missing, they'll return 'error' frames/chunks.
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void Sequence::updateTracks()
{
    int index = 0;
    mVideoTrackMap.clear();
    for ( TrackPtr track : mVideoTracks )
    {
        track->setIndex(index);
        mVideoTrackMap[index] = track;
        ++index;
    }
    index = 0;
    mAudioTrackMap.clear(); // PERF: to cache
    for ( TrackPtr track : mAudioTracks )
    {
        track->setIndex(index);
        mAudioTrackMap[index] = track;
        ++index;
    }

    ASSERT(!mVideoTracks.empty()); // Avoid problems with sequences that have no tracks. Example:
    ASSERT(!mAudioTracks.empty()); // Drag from projectview to a sequence without tracks: crash in drag handling.
}

void Sequence::updateLength()
{
    pts maxlength = 0;
    for ( TrackPtr track : getTracks() )
    {
        maxlength = std::max(track->getLength(), maxlength);
    }

    if (maxlength != mCache.length)
    {
        ProcessEvent(EventLengthChanged(maxlength)); // Handled immediately
        mCache.length = maxlength;
    }
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, const Sequence& obj)
{
    os << &obj                           << '|'
        << static_cast<const Node&>(obj) << '|'
        << obj.mDividerPosition          << '|'
        << obj.mVideoTracks              << '|'
        << obj.mAudioTracks              << '|'
        << obj.mVideoPosition            << '|'
        << obj.mRender;
    return os;
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void Sequence::serialize(Archive & ar, const unsigned int version)
{
    try
    {
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(IAudio);
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Node);
        ar & BOOST_SERIALIZATION_NVP(mName);
        ar & BOOST_SERIALIZATION_NVP(mDividerPosition);
        ar & BOOST_SERIALIZATION_NVP(mVideoTracks);
        ar & BOOST_SERIALIZATION_NVP(mAudioTracks);
        if (Archive::is_loading::value)
        {
            for ( TrackPtr track : mVideoTracks )
            {
                track->Bind(model::EVENT_LENGTH_CHANGED, &Sequence::onTrackLengthChanged, this);
            }
            for ( TrackPtr track : mAudioTracks )
            {
                track->Bind(model::EVENT_LENGTH_CHANGED, &Sequence::onTrackLengthChanged, this);
            }
            updateTracks();
            updateLength();

            LOG_INFO << dump(boost::dynamic_pointer_cast<Sequence>(shared_from_this()));
        }
        ar & BOOST_SERIALIZATION_NVP(mRender);
    }
    catch (boost::archive::archive_exception& e) { VAR_ERROR(e.what());                         throw; }
    catch (boost::exception &e)                  { VAR_ERROR(boost::diagnostic_information(e)); throw; }
    catch (std::exception& e)                    { VAR_ERROR(e.what());                         throw; }
    catch (...)                                  { LOG_ERROR;                                   throw; }
}
template void Sequence::serialize<boost::archive::xml_oarchive>(boost::archive::xml_oarchive& ar, const unsigned int archiveVersion);
template void Sequence::serialize<boost::archive::xml_iarchive>(boost::archive::xml_iarchive& ar, const unsigned int archiveVersion);

} //namespace

BOOST_CLASS_EXPORT_IMPLEMENT(model::Sequence)