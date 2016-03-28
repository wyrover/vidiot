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

#include "Part.h"

namespace gui { namespace timeline { namespace cmd {
    class ExecuteDrop;
    struct Drop;
    typedef std::vector<Drop> Drops;
}}}

namespace gui { namespace timeline {
class DummyView;

class ShiftParams;
typedef boost::shared_ptr<ShiftParams> Shift;

class Drag
    :   public Part
{
public:

    static const pixel Threshold;

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION METHODS
    //////////////////////////////////////////////////////////////////////////

    Drag(Timeline* timeline);
    virtual ~Drag();

    //////////////////////////////////////////////////////////////////////////
    // START/STOP
    //////////////////////////////////////////////////////////////////////////

    /// Must be called whenever a new drag operation is initiated.
    /// \param external true if the dragged clips originate from an external source (project view, file system)
    void start(const wxPoint& hotspot, bool external);

    /// Must be called to show the current drag image. Is required when starting
    /// a new drag, but also when the drag needs to be redisplayed (for instance,
    /// as a result of zooming).
    void show();

    void toggleSnapping(); ///< Temporarily disable/enable snap-to-whatever

    /// Move the drag image to the given position.
    /// \param position move the mouse pointer to this position
    void move(wxPoint position);

    /// Execute the drop.
    void drop();

    /// Stop the drag operation, to be called after a drop has been executed or
    /// the drag operation has been canceled.
    void stop();

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    bool isActive() const; ///< \return true if a drag operation with the currently selected clips is in effect.

    /// \return true if the given clip is currently being dragged
    /// \param clip clip to be checked
    bool contains(const model::IClipPtr& clip) const;

    /// \return size of the current drag bitmap
    wxSize getBitmapSize() const;

    /// \return position of the current drag bitmap
    wxPoint getBitmapPosition() const;

    /// \return the current shift to be applied
    Shift getShift() const;

    pts getSnapOffset() const;

    //////////////////////////////////////////////////////////////////////////
    // DRAW
    //////////////////////////////////////////////////////////////////////////

    void updateDragBitmap();
    void drawDraggedClips(wxDC& dc, const wxRegion& region, const wxPoint& offset) const;
    void drawSnaps(wxDC& dc, const wxRegion& region, const wxPoint& offset) const;

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    cmd::ExecuteDrop* mCommand = nullptr;       ///< The command that is submitted when the drop operation finishes.
    wxPoint mHotspot;                           ///< Hotspot within the timeline. Basically: pointer position at start of dragging.
    pts mHotspotPts = 0;                        ///< The pts value that corresponds to the hotspot's x position. Required when changing zoom/scrolling.
    wxPoint mPosition;                          ///< Current pointer drag position. In timeline coordinates.
    wxBitmap mBitmap;                           ///< The bitmap containing the dragged clips. It is reduced to 'only visible area'.
    wxPoint mBitmapOffset;                      ///< This offset ensures that correct areas can be used when positioning on the timeline.
    bool mActive = false;                       ///< True if dragging is currently active.
    std::vector<pts> mSnapPoints;               ///< Sorted list containing all possible 'snap to' points (pts values). Filled upon start of drag.
    std::vector<pts> mDragPoints;               ///< Sorted list containing all possible 'snapping' points (pts values) in the dragged area. Filled upon start of drag.
    pts mSnapOffset = 0;                        ///< Resulting offset caused by 'snapping to' a clip
    std::vector<pts> mSnaps;                    ///< List of current snapping positions (that is, where one of the dragged clips 'touches' the pts position of another clip)
    Shift mShift;                               ///< Uninitialized if no shift active. When initialized holds info on the current shift.
    bool mSnappingEnabled = false;              ///< Used to overrule snapping during a drag operation.

    //////////////////////////////////////////////////////////////////////////
    // DRAGINFO
    //////////////////////////////////////////////////////////////////////////

    /// This class is used to hide if-then-else constructs for distinguishing between
    /// video and audio tracks as much as possible. It provides one entry point to the
    /// list of tracks, and abstracts away if this is about video or about audio tracks.
    class DragInfo
        :   public Part
    {
    public:
        int mOffset = 0;    ///< Offset by which to draw dragged tracks. Note that for 'outside' drags, it contains the id over which the new assets are being dragged.
        int mMinOffset = 0; ///< Min allowed value for offset (to avoid that tracks are moved into the void)
        int mMaxOffset = 0; ///< Mix allowed value for offset (to avoid that tracks are moved into the void)

        /// Default constructor
        DragInfo(Timeline* timeline, bool isVideo);

        virtual ~DragInfo();

        /// Reset all offsets when a new drag operation is started
        void reset();

        /// Update the offset of tracks of this type, given a track in the timeline and a dragged track
        /// that is currently dragged above that track in the timeline.
        /// \param indexOfTrackInTimeline index of a track in the timeline
        /// \param indexOfDraggedTrack index of the track that is currently 'above' the track in the timeline
        void updateOffset(int indexOfTrackInTimeline, int indexOfDraggedTrack);

        /// \return true if the offset is such that an additional track is required to hold the clips.
        bool requiresAddedTrack() const;

        /// \return audio or video track with given index.
        /// Whether it's an audio or a video track returned depends on the scope of this DragInfo object.
        model::TrackPtr getTrack(int index);

        /// \param track temporary track used for 'outside' drags
        void setTempTrack(const model::TrackPtr& track);

        /// \return temporary track used for 'outside' drags
        model::TrackPtr getTempTrack();

        model::TrackPtr trackOnTopOf(const model::TrackPtr& track);
        model::TrackPtr trackUnder(const model::TrackPtr& draggedtrack);

        int nTracks(); ///< @return number of tracks of this type currently in the timeline

    private:

        friend std::ostream& operator<<(std::ostream& os, const DragInfo& obj);

        model::TrackPtr mTempTrack = nullptr;   ///< For 'outside' drags, holds the temporary track that contains the new assets
        bool mIsVideo = false;                  ///< true if this object applies to about video tracks, false if this object applies to audio tracks
        DummyView* mView = nullptr;             ///< This view can be used as a container for temporary tracks used for drawing a 'outside' drag operation
    };

    DragInfo mVideo;
    DragInfo mAudio;

    model::TrackPtr mDraggedTrack = nullptr;    ///< Holds, when dragging, the track directly under the mouse pointer when starting the drag (the track which is dragged)
    model::TrackPtr mDropTrack = nullptr;       ///< Holds, when dragging, the track directly under the mouse pointer when dragging (the track onto which is dropped)

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    /// Reset all members used for the dragging process.
    void reset();

    /// \return the track that is currently dragged on top of 'track'
    /// \return 0 if no track is found on top of this track
    /// \param track a track in the timeline
    /// Note that the returned track may be completely unselected, meaning
    /// that actually no clips from that track are visually dragged.
    model::TrackPtr trackOnTopOf(const model::TrackPtr& track);

    /// \return the track that is currently 'under' draggedtrack
    model::TrackPtr trackUnder(const model::TrackPtr& draggedtrack);

    /// \return the DragInfo object that corresponds to the given track.
    /// \param track track that indicates audio or video
    /// Given the track, either mVideo or mAudio is returned.
    DragInfo& getAssociatedInfo(const model::TrackPtr& track);

    /// Update the offset of tracks of the given track's type.
    void updateOffset(const model::TrackPtr& trackUnderPointer);

    /// Update the currently dragged track. The dragged track can be updated
    /// by holding SHIFT (move grab point) or by moving from audio to video
    /// or vice versa.
    /// May be called with a 0-pointer. In that case, nothing is changed.
    /// \param track track (in the timeline) on top of which the mouse pointer currently is.
    void updateDraggedTrack(const model::TrackPtr& track);

    /// Return the current position of the drag. That is, the difference between
    /// the original hotspot position and the current hotspot position.
    /// \return bitmap offset in pixels
    wxPoint getDraggedDistance() const;

    /// Return the dragged distance in pts value
    pts getDraggedPtsDistance() const;

    /// Return the current leftmost pts value of the dragged clips
    pts getDragPtsPosition() const;

    /// Return the current position of a dragged point
    pts getDraggedPosition(pts dragpoint) const;

    /// Return the size of the dragged clips
    pts getDragPtsSize() const;

    /// \return the amount of pixels the drag is moved because of snapping
    wxPoint getSnapPixels() const;

    /// Determine if there is a close match between a timeline cut and a cut
    /// in the dragged clips. Will update mPosition so that the dragged object
    /// is 'snapped to' that cut.
    void determineSnapOffset();

    /// Fill mPossibleSnapPoints with a list of possible 'snap to' points.
    /// Basically, these are lists of all the cuts in all the tracks in the timeline
    /// (excluding the selected==dragged clips).
    /// This is done at the start of a drag only, for performance reasons.
    /// This is also done when a shift is applied to make room for dragged clips.
    void determinePossibleSnapPoints();

    /// Fill mPossibleDragPoints with a list of possible 'snap to' points in the dragged clips.
    /// Basically, these are lists of all the cuts in the dragged area (thus, the selected clips).
    /// This is done at the start of a drag only, for performance reasons.
    void determinePossibleDragPoints();

    /// Determine the shift to be applied to 'fit' the dragged clips in between the clips
    /// it is currently hovering over
    void determineShift();

    /// Return the list of 'drops' on the given track
    /// \param track onto which clips are dropped
    /// \return list of drops onto that track
    cmd::Drops getDrops(const model::TrackPtr& track);

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<<(std::ostream& os, const Drag& obj);
    friend std::ostream& operator<<(std::ostream& os, const DragInfo& obj);
};

std::ostream& operator<<(std::ostream& os, const Drag& obj);
std::ostream& operator<<(std::ostream& os, const Drag::DragInfo& obj);
}} // namespace
