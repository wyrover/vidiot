// Copyright 2013-2015 Eric Raijmakers.
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
#include "IClip.h"
#include "ModelEvent.h"
#include "NodeEvent.h"
#include "ProjectModification.h"
#include "Properties.h"
#include "Render.h"
#include "SequenceEvent.h"
#include "UtilClone.h"
#include "UtilLog.h"
#include "UtilLogStl.h"
#include "UtilLogWxwidgets.h"
#include "UtilSerializeWxwidgets.h"
#include "UtilSet.h"
#include "UtilVector.h"
#include "VideoComposition.h"
#include "VideoCompositionParameters.h"
#include "VideoTrack.h"
#include "Window.h"

namespace model {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

Sequence::Sequence()
    : wxEvtHandler()
    , IControl()
    , IVideo()
    , IAudio()
    , Node()
    , mName()
    , mDividerPosition(0)
    , mVideoTracks()
    , mAudioTracks()
    , mVideoTrackMap()
    , mAudioTrackMap()
    , mVideoPosition(0)
    , mAudioPosition(0)
    , mRender()
{
    VAR_DEBUG(this);
    // Serialization will fill in the members
    // and call updateTracks()
}

Sequence::Sequence(const wxString& name)
    : wxEvtHandler()
    , IControl()
    , IVideo()
    , IAudio()
    , Node()
    , mName(name)
    , mDividerPosition(0)
    , mVideoTracks()
    , mAudioTracks()
    , mVideoTrackMap()
    , mAudioTrackMap()
    , mVideoPosition(0)
    , mAudioPosition(0)
    , mRender()
{
    VAR_DEBUG(this);
    mVideoTracks.push_back(boost::make_shared<VideoTrack>());
    mAudioTracks.push_back(boost::make_shared<AudioTrack>());
    updateTracks();
}

Sequence::Sequence(const Sequence& other)
    : wxEvtHandler()
    , IControl()
    , IVideo()
    , IAudio()
    , Node()
    , mName(other.mName)
    , mDividerPosition(other.mDividerPosition)
    , mVideoTracks(make_cloned<Track>(other.mVideoTracks))
    , mAudioTracks(make_cloned<Track>(other.mAudioTracks))
    , mVideoTrackMap() // Duplicate administration left empty! (This constructor should only be used for cloning directly before rendering)
    , mAudioTrackMap()  // Duplicate administration left empty! (...and for rendering the duplicate administration is not required)
    , mVideoPosition(0)
    , mAudioPosition(0)
    , mRender(make_cloned<render::Render>(other.mRender))
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
    for ( TrackPtr track : getTracks() )
    {
        nFrames = std::max<pts>(nFrames, track->getLength());
    }
    return nFrames;
}

void Sequence::moveTo(pts position)
{
    VAR_DEBUG(position);
    mVideoPosition = position;
    mAudioPosition = position;
    for ( TrackPtr track : getTracks() )
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
    for ( TrackPtr track : getTracks() )
    {
        track->clean();
    }
}

//////////////////////////////////////////////////////////////////////////
// IVIDEO
//////////////////////////////////////////////////////////////////////////

VideoFramePtr Sequence::getNextVideo(const VideoCompositionParameters& parameters)
{
    VideoCompositionPtr composition = getVideoComposition(VideoCompositionParameters(parameters).setPts(mVideoPosition));
    VideoFramePtr videoFrame = composition->generate();
    mVideoPosition++;
    return videoFrame;
}

//////////////////////////////////////////////////////////////////////////
// IAUDIO
//////////////////////////////////////////////////////////////////////////

AudioChunkPtr Sequence::getNextAudio(const AudioCompositionParameters& parameters)
{
    AudioCompositionPtr composition = getAudioComposition(AudioCompositionParameters(parameters).setPts(mAudioPosition).determineChunkSize());
    AudioChunkPtr audioChunk = composition->generate();
    mAudioPosition++;
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
         track->Bind(model::EVENT_HEIGHT_CHANGED, &Sequence::onTrackHeightChanged, this);
    }

    UtilVector<TrackPtr>(mVideoTracks).addElements(tracks,position);

    updateTracks();

    // ProcessEvent is used. Model events must be processed synchronously to avoid inconsistent states in
    // the receivers of these events (typically, the view classes in the timeline).
    // Example: Create sequence from autofolder.
    // 1. Sequence is created
    // 2. Tracks are added (and if queue events were used these are queued here)
    // 3. Sequence is opened and initial tracks are 'added' as views
    // 4. At some later point, the queued events result in 'double' views for the added tracks.
    model::EventAddVideoTracks event(TrackChange(tracks, position));
    ProcessEvent(event);

    EventHeightChanged heightEvent(-1);
    ProcessEvent(heightEvent);

    // This may NOT be called before the add/remove event is sent: updateLength() may cause view updates,
    // which cause accesses to the model. By that time, all views must know the proper list of tracks.
    updateLength();
}

void Sequence::addAudioTracks(const Tracks& tracks, const TrackPtr& position)
{
    for ( model::TrackPtr track : tracks )
    {
         track->Bind(model::EVENT_LENGTH_CHANGED, &Sequence::onTrackLengthChanged, this);
         track->Bind(model::EVENT_HEIGHT_CHANGED, &Sequence::onTrackHeightChanged, this);
    }

    UtilVector<TrackPtr>(mAudioTracks).addElements(tracks,position);

    updateTracks();

    // ProcessEvent is used. Model events must be processed synchronously to avoid inconsistent states in
    // the receivers of these events (typically, the view classes in the timeline). Example: See addVideoTracks.
    model::EventAddAudioTracks event(TrackChange(tracks, position));
    ProcessEvent(event);

    EventHeightChanged heightEvent(-1);
    ProcessEvent(heightEvent);

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
        track->Unbind(model::EVENT_HEIGHT_CHANGED, &Sequence::onTrackHeightChanged, this);
    }
    TrackPtr position = UtilVector<TrackPtr>(mVideoTracks).removeElements(tracks);

    updateTracks();

    // ProcessEvent is used. Model events must be processed synchronously to avoid inconsistent states in
    // the receivers of these events (typically, the view classes in the timeline). Example: See addVideoTracks.
    model::EventRemoveVideoTracks event(TrackChange(Tracks(),TrackPtr(),tracks, position));
    ProcessEvent(event);

    EventHeightChanged heightEvent(-1);
    ProcessEvent(heightEvent);

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
        track->Unbind(model::EVENT_HEIGHT_CHANGED, &Sequence::onTrackHeightChanged, this);
    }
    TrackPtr position = UtilVector<TrackPtr>(mAudioTracks).removeElements(tracks);

    updateTracks();

    // ProcessEvent is used. Model events must be processed synchronously to avoid inconsistent states in
    // the receivers of these events (typically, the view classes in the timeline). Example: See addVideoTracks.
    model::EventRemoveAudioTracks event(TrackChange(Tracks(),TrackPtr(),tracks, position));
    ProcessEvent(event);

    EventHeightChanged heightEvent(-1);
    ProcessEvent(heightEvent);

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

Tracks Sequence::getTracks() const
{
    Tracks tracks;
    tracks.insert(tracks.end(), mVideoTracks.begin(), mVideoTracks.end());
    tracks.insert(tracks.end(), mAudioTracks.begin(), mAudioTracks.end());
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
    if (mDividerPosition != position)
    {
        mDividerPosition = position;
        model::ProjectModification::trigger();
        EventHeightChanged event(-1);
        ProcessEvent(event); // Handled immediately
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
        composition->add(boost::dynamic_pointer_cast<IAudio>(track)->getNextAudio(parameters));
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

bool Sequence::isEmptyAt(pts position ) const
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


void Sequence::onTrackHeightChanged(EventHeightChanged& event)
{
    ProcessEvent(event); // Handled immediately
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
    model::EventRenameNode event(NodeWithNewName(self(),mName));
    gui::Window::get().ProcessModelEvent(event);
}

void Sequence::check(bool immediately)
{
    // Nothing is checked: If files are missing, they'll return 'error' frames/chunks.
}

bool Sequence::hasSequences() const
{
    return true;
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
        EventLengthChanged event(maxlength);
        ProcessEvent(event); // Handled immediately
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
        << obj.mAudioPosition            << '|'
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
                track->Bind(model::EVENT_HEIGHT_CHANGED, &Sequence::onTrackHeightChanged, this);
                track->onLoad();
            }
            for ( TrackPtr track : mAudioTracks )
            {
                track->Bind(model::EVENT_LENGTH_CHANGED, &Sequence::onTrackLengthChanged, this);
                track->Bind(model::EVENT_HEIGHT_CHANGED, &Sequence::onTrackHeightChanged, this);
                track->onLoad();
            }
            updateTracks();
            updateLength();
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
