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

#include "IControl.h"
#include "UtilRTTI.h"

namespace model {

class IClip
    :   public wxEvtHandler // MUST BE FIRST INHERITED CLASS FOR WXWIDGETS EVENTS TO BE RECEIVED.
    ,   public IControl
    ,   public IRTTI
    ,   public boost::enable_shared_from_this<IClip>
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    IClip() {};
	virtual ~IClip() {};

    virtual IClip* clone() const = 0;
    virtual void onCloned() = 0;

    //////////////////////////////////////////////////////////////////////////
    // TRACK
    //////////////////////////////////////////////////////////////////////////

    /// \return the track in which this clip is contained. A null ptr is returned if the clip is not in a track.
    virtual TrackPtr getTrack() const = 0;
    virtual bool hasTrack() const = 0;

    /// \return the index (if any) in the track of this clip
    /// \pre hasTrack()
    virtual int getIndex() const = 0;

    /// \return pts (in containing track) of begin point of clip.
    /// The frame at this position is the first frame of this clip.
    /// The frames of a clip are [ getLeftPts,getRightPts )
    virtual pts getLeftPts() const = 0;

    /// \return pts (in containing track) AFTER end point of clip.
    /// The frame at this position is AFTER the last frame of this clip
    /// The frames of a clip are [ getLeftPts,getRightPts )
    virtual pts getRightPts() const = 0;

    /// \return next clip in track. IClipPtr() if there is none.
    virtual IClipPtr getNext() = 0;

    /// \return previous clip in track. IClipPtr() if there is none.
    virtual IClipPtr getPrev() = 0;

    /// \return next clip in track. IClipPtr() if there is none.
    virtual ConstIClipPtr getNext() const = 0;

    /// \return previous clip in track. IClipPtr() if there is none.
    virtual ConstIClipPtr getPrev() const = 0;

    //////////////////////////////////////////////////////////////////////////
    // LINK
    //////////////////////////////////////////////////////////////////////////

    virtual void setLink(const IClipPtr& link) = 0;
    virtual IClipPtr getLink() const = 0;

    //////////////////////////////////////////////////////////////////////////
    // POSITION
    //////////////////////////////////////////////////////////////////////////

    /// \return Minimum allowed value for adjustBegin given the available data.
    /// \note This takes into account any spare room that must be kept for adjacent transitions/clips
    /// \post getMinAdjustBegin() <= 0
    virtual pts getMinAdjustBegin() const = 0;

    /// \return Maximum allowed value for adjustBegin given the available data.
    /// \note This takes into account any spare room that must be kept for adjacent transitions/clips
    /// \post getMaxAdjustBegin() >= 0
    virtual pts getMaxAdjustBegin() const = 0;

    /// If adjustment is positive then move the begin point of the clip backwards
    /// in time (increase the start pts). If adjustment is negative then move the
    /// begin point of the clip forward in time (decrease the start pts).
    /// \param adjustment pts count to add/subtract from the begin point
    /// \pre clip is not part of a track
    /// \pre adjustment >= getMinAdjustBegin()
    /// \pre adjustment <= getMaxAdjustBegin()
    virtual void adjustBegin(pts adjustment) = 0;

    /// \return Minimum allowed value for adjustEnd given the available data.
    /// \note This takes into account any spare room that must be kept for adjacent transitions/clips
    /// \post getMinAdjustEnd() <= 0
    virtual pts getMinAdjustEnd() const = 0;

    /// \return Maximum allowed value for adjustEnd given the available data.
    /// \note This takes into account any spare room that must be kept for adjacent transitions/clips
    /// \post getMaxAdjustEnd() >= 0
    virtual pts getMaxAdjustEnd() const = 0;

    /// Set the new length of the clip.
    /// \param adjustment pts count to add/subtract from the length
    /// \pre clip is not part of a track
    /// \pre adjustment >= getMinAdjustEnd()
    /// \pre adjustment <= getMaxAdjustEnd()
    virtual void adjustEnd(pts adjustment) = 0;

    //////////////////////////////////////////////////////////////////////////
    // ADJACENT TRANSITION HANDLING
    //////////////////////////////////////////////////////////////////////////

    /// Return the transition left of this clip, but only if that transition actually applies to this clip.
    /// \return 'in' transition for this clip, 0 if there is no transition snooping away frames from this clip
    virtual TransitionPtr getInTransition() const = 0;

    /// Return the transition right of this clip, but only if that transition actually applies to this clip.
    /// \return 'out' transition for this clip, 0 if there is no transition snooping away frames from this clip
    virtual TransitionPtr getOutTransition() const = 0;

    /// Return the length of the clip as viewed by the user.
    /// The difference between the actual length of the clip (getLength()) and
    /// this perceived length is the area of the clip that is part of one of
    /// its adjacent transitions; basically, the part of the clip that is shown
    /// 'under' a transition in the timeline.
    virtual pts getPerceivedLength() const = 0;

    /// Return the left position of the clip as viewed by the user.
    /// The difference between the actual position of the clip (getLeftpts()) and
    /// this perceived position is the area of the clip that is part of an
    /// adjacent in transition; basically, the part of the clip that is shown
    /// 'under' a transition in the timeline.
    virtual pts getPerceivedLeftPts() const = 0;

    /// Return the right position of the clip as viewed by the user.
    /// The difference between the actual position of the clip (getRightpts()) and
    /// this perceived position is the area of the clip that is part of an
    /// adjacent out transition; basically, the part of the clip that is shown
    /// 'under' a transition in the timeline.
    virtual pts getPerceivedRightPts() const = 0;

    /// \return a clone that includes that area required for adjacent transitions
    virtual IClipPtr getExtendedClone() const = 0;

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    virtual bool getSelected() const = 0;           ///< /return true if this clip is selected
    virtual void setSelected(bool selected) = 0;    ///< Select or deselect clip

    virtual bool getDragged() const = 0;           ///< /return true if this clip is being dragged
    virtual void setDragged(bool dragged) = 0;     ///< Set dragged value for clip

    /// \return list of all cuts for the clip
    /// \param exclude list of clips for which the cuts should not be added
    virtual std::set<pts> getCuts(const std::set<IClipPtr>& exclude = std::set<IClipPtr>()) const = 0;

    /// Dump the contents of the clip into the stream. Used for generating logging.
    /// \return the stream again
    virtual std::ostream& dump(std::ostream& os) const = 0;

    /// \return a five character long string representation of the clip type, for logging.
    virtual const char* getType() const = 0;

    //////////////////////////////////////////////////////////////////////////
    // ACCESS DATA GENERATOR
    //////////////////////////////////////////////////////////////////////////

    /// \return the file that provides the actual data
    /// Returns 0 if there is no such file (for instance, empty clip/transition)
    virtual FilePtr getFile() const = 0;

protected:

    //////////////////////////////////////////////////////////////////////////
    // COPY CONSTRUCTOR
    //////////////////////////////////////////////////////////////////////////

    /// Copy constructor. Use make_cloned for making deep copies of objects.
    /// \note the clone is not automatically part of the track!!!
    /// \see make_cloned
    IClip(const IClip& other);

    //////////////////////////////////////////////////////////////////////////
    // CURRENT POSITION HANDLING
    //////////////////////////////////////////////////////////////////////////

    /// This method resets mNewStartPosition. This must be called whenever there
    /// is new playback progress.
    virtual void invalidateNewStartPosition() = 0;

    /// Return the most recent position as specified in moveTo(). This is
    /// uninitialized when there was playback progress after the moveTo.
    /// \see invalidateNewStartPosition
    virtual boost::optional<pts> getNewStartPosition() const = 0;
};

} // namespace
