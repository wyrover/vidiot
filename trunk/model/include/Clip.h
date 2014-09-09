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

#ifndef MODEL_CLIP_H
#define MODEL_CLIP_H

#include "IClip.h"

namespace model {

class Clip
    :   public IClip
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    Clip();                   ///< Constructor for recovery from disk

    virtual ~Clip();

    virtual void onCloned() override;

    //////////////////////////////////////////////////////////////////////////
    // ICONTROL
    //////////////////////////////////////////////////////////////////////////

    virtual wxString getDescription() const override;
    virtual void clean() override;

    //////////////////////////////////////////////////////////////////////////
    // ICLIP
    //////////////////////////////////////////////////////////////////////////

    TrackPtr getTrack() override;
    bool hasTrack() const override;

    IClipPtr getNext() override;
    IClipPtr getPrev() override;
    ConstIClipPtr getNext() const override;
    ConstIClipPtr getPrev() const override;

    pts getLeftPts() const override;
    pts getRightPts() const override;

    virtual void setLink(const IClipPtr& link) override;
    IClipPtr getLink() const override;

    TransitionPtr getInTransition() const override;
    TransitionPtr getOutTransition() const override;
    pts getPerceivedLength() const override;

    bool getSelected() const override;
    void setSelected(bool selected) override;

    bool getDragged() const override;
    void setDragged(bool dragged) override;

    void invalidateNewStartPosition() override;
    boost::optional<pts> getNewStartPosition() const override;
    void setNewStartPosition(pts position);
    virtual std::set<pts> getCuts(const std::set<IClipPtr>& exclude = std::set<IClipPtr>()) const override;

protected:

    //////////////////////////////////////////////////////////////////////////
    // COPY CONSTRUCTOR
    //////////////////////////////////////////////////////////////////////////

    /// Copy constructor. Use make_cloned for making deep copies of objects.
    /// \note the clone is not automatically part of the track!!!
    /// \see make_cloned
    Clip(const Clip& other);

private:

    //////////////////////////////////////////////////////////////////////////
    // SET TRACK INFORMATION
    //////////////////////////////////////////////////////////////////////////

    friend class Track;
    void setTrackInfo(
        const TrackPtr& track = TrackPtr(),
        const IClipPtr& prev = IClipPtr(),
        const IClipPtr& next = IClipPtr(),
        pts trackPosition = 0,
        const unsigned int& index = 0);

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    WeakTrackPtr mTrack;    ///< Track which holds this clip. 0 if current clip is not part of a track. Stored as weak_ptr to avoid cyclic dependencies (leading to memory leaks).
    WeakIClipPtr mPrev;     ///< Previous clip in the track. 0 if current clip is not part of a track. Stored as weak_ptr to avoid cyclic dependencies (leading to memory leaks).
    WeakIClipPtr mNext;     ///< Next clip in the track. 0 if current clip is not part of a track. Stored as weak_ptr to avoid cyclic dependencies (leading to memory leaks).
    pts mLeftPtsInTrack;    ///< Position inside the track. 0 if not in a track.
    unsigned int mIndex;    ///< Index of this clip in the track (for debugging)

    WeakIClipPtr mLink;     ///< Clip that this clip is linked with. Stored as weak_ptr to avoid circular dependency between two linked clips which causes memory leaks.

    boost::optional<pts> mNewStartPosition;  ///< The most recent position as specified in 'moveTo()'.
    pts mGeneratedPts;                      ///< (approximate) pts value of last video/audio returned with getNext*

    bool mSelected;                         ///< True if this clip is currently selected
    bool mDragged;                          ///< True if this clip is currently dragged

    mutable wxString mDescription;  ///< Stored for performance (cached) and for easier debugging.

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<<(std::ostream& os, const Clip& obj);

    //////////////////////////////////////////////////////////////////////////
    // SERIALIZATION
    //////////////////////////////////////////////////////////////////////////

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);
};
} // namespace

// Workaround needed to prevent compile-time errors (mpl_assertion_in_line...) with gcc
//#include  <boost/preprocessor/slot/counter.hpp>
//#include BOOST____PP_UPDATE_COUNTER()
//#line BOOST_____PP_COUNTER
BOOST_CLASS_VERSION(model::Clip, 2)
BOOST_CLASS_EXPORT_KEY(model::Clip)

#endif
