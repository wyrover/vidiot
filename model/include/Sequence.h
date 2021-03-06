// Copyright 2013-2016 Eric Raijmakers.
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

#pragma once

#include "IAudio.h"
#include "IControl.h"
#include "IVideo.h"
#include "Node.h"

namespace model {

class EventLengthChanged;
class EventHeightChanged;

/// Sequences must always have one or more video tracks and one or more audio tracks.
/// That is done to have to check (for instance in timeline::Drag.cpp) whether there
/// are such tracks available in the sequence.
class Sequence
    :   public wxEvtHandler // MUST BE FIRST INHERITED CLASS FOR WXWIDGETS EVENTS TO BE RECEIVED.
    ,   public IControl
    ,   public IVideo
    ,   public IAudio
    ,   public Node
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    Sequence();

    Sequence(const wxString& name);

    /// Copy constructor. Use make_cloned for making deep copies of objects.
    ///
    /// \return a clone of the sequence, that can be used for rendering
    /// The sequence is cloned just before the rendering is started. That ensures
    /// that the sequence can be edited further, while the 'previous version' is
    /// being rendered. This clone does not need to copy all attributes, since the
    /// only action done with it is rendering. In fact, only the minimal cloning
    /// should be done, for performance reasons.
    ///
    /// This copy constructor may only be used for making a copy for rendering
    /// as all duplicate administration (the one kept for performance) is NOT
    /// duplicated. In fact, that couldn't be duplicated because of the
    /// shared_from_this() usage (and that can't be called in constructors yet).
    /// See Track::updateClips() for that shared_from_this() usage.
    ///
    /// \see make_cloned
    Sequence(const Sequence& other);

    virtual Sequence* clone() const;

    virtual void onCloned();

    virtual ~Sequence();

    //////////////////////////////////////////////////////////////////////////
    // ICONTROL
    //////////////////////////////////////////////////////////////////////////

    virtual pts getLength() const override;
    virtual void moveTo(pts position) override;
    virtual wxString getDescription() const override;
    virtual void clean() override;

    //////////////////////////////////////////////////////////////////////////
    // IVIDEO
    //////////////////////////////////////////////////////////////////////////

    virtual VideoFramePtr getNextVideo(const VideoCompositionParameters& parameters) override;

    //////////////////////////////////////////////////////////////////////////
    // IAUDIO
    //////////////////////////////////////////////////////////////////////////

    virtual AudioChunkPtr getNextAudio(const AudioCompositionParameters& parameters) override;

    //////////////////////////////////////////////////////////////////////////
    // SEQUENCE SPECIFIC
    //////////////////////////////////////////////////////////////////////////

    void addVideoTracks(const Tracks& tracks, const TrackPtr& position = TrackPtr());
    void addAudioTracks(const Tracks& tracks, const TrackPtr& position = TrackPtr());
    void removeVideoTracks(const Tracks& tracks);
    void removeAudioTracks(const Tracks& tracks);

    Tracks getVideoTracks();
    Tracks getAudioTracks();
    Tracks getTracks() const;

    TrackPtr getVideoTrack(int index);
    TrackPtr getAudioTrack(int index);

    pixel getDividerPosition() const;
    void setDividerPosition(pixel position);

    std::set<IClipPtr> getSelectedClips();

    VideoCompositionPtr getVideoComposition(const VideoCompositionParameters& parameters);

    AudioCompositionPtr getAudioComposition(const AudioCompositionParameters& parameters);

    ///\return list of all cuts in the sequence
    /// Each begin and end of a clip is returned as a clip. Note that this includes the begin and end of each transition, but not (yet) the cut 'under' the transition.
    std::set<pts> getCuts(const std::set<IClipPtr>& exclude = std::set<IClipPtr>());

    bool isEmptyAt(pts position ) const; ///< \return true if the sequence holds only emptyness at the given position

    void onTrackLengthChanged(EventLengthChanged& event);
    void onTrackHeightChanged(EventHeightChanged& event);

    //////////////////////////////////////////////////////////////////////////
    // RENDERING
    //////////////////////////////////////////////////////////////////////////

    render::RenderPtr getRender();
    void setRender(const render::RenderPtr& render);

    //////////////////////////////////////////////////////////////////////////
    // INODE
    //////////////////////////////////////////////////////////////////////////

    wxString    getName() const override;
    void        setName(const wxString& name) override;
    void        check(bool immediately = false) override;
    bool        hasSequences() const override;

private:

    wxString mName;
    Tracks mVideoTracks;
    Tracks mAudioTracks;
    std::map<int, TrackPtr> mVideoTrackMap;
    std::map<int, TrackPtr> mAudioTrackMap;
    pixel mDividerPosition = 0;
    pts mVideoPosition = 0;
    pts mAudioPosition = 0;
    render::RenderPtr mRender = nullptr;

    /// Some variables are only required for better performance.
    /// These contain 'duplicate/redundant' information.
    struct Cache
    {
        Cache()
            : length(0)
        {
        }
        pts length;
    };
    Cache mCache;

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    /// Update the various tracks upon insertion/removal etc.
    void updateTracks();

    /// Updates the cached sequence length. Sends an event if the length has changed.
    /// May cause new accesses to the model. Thus, ensure that the model contents
    /// (and any changes thereof) have been signaled to the view classes.
    void updateLength();

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<<(std::ostream& os, const Sequence& obj);

    //////////////////////////////////////////////////////////////////////////
    // SERIALIZATION
    //////////////////////////////////////////////////////////////////////////

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);
};

} // namespace

BOOST_CLASS_VERSION(model::Sequence, 1)
BOOST_CLASS_EXPORT_KEY(model::Sequence)
