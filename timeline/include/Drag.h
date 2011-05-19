#ifndef DRAG_H
#define DRAG_H

#include <list>
#include <wx/bitmap.h>
#include <wx/dcmemory.h>
#include "Part.h"
#include "ExecuteDrop.h"

namespace model {
    class Track;
    typedef boost::shared_ptr<Track> TrackPtr;
    typedef std::list<TrackPtr> Tracks;
}

namespace gui { namespace timeline {

class DummyView;

class Drag
    :   public Part
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION METHODS
    //////////////////////////////////////////////////////////////////////////

    Drag(Timeline* timeline);
    ~Drag();
	
    //////////////////////////////////////////////////////////////////////////
    // START/STOP
    //////////////////////////////////////////////////////////////////////////

    /// Must be called whenever a new drag operation is initiated.
    /// \param isInsideDrag true if this is a drag within the timeline, false if there are new clips being dragged into the timeline (from the project view)
    void start(wxPoint hotspot, bool isInsideDrag);

    /// Must be called to show the current drag image. Is required when starting
    /// a new drag, but also when the drag needs to be redisplayed (for instance,
    /// as a result of zooming).
    void show();

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
    
    bool isActive() const; ///< @return true if a drag operation with the currently selected clips is in effect.

    /// \return true if the given clip is currently being dragged
    /// \param clip clip to be checked
    bool contains(model::IClipPtr clip) const;

    //////////////////////////////////////////////////////////////////////////
    // DRAW
    //////////////////////////////////////////////////////////////////////////

    wxBitmap getDragBitmap();
    void draw(wxDC& dc) const;

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    bool mIsInsideDrag;                 ///< True: drag&drop within the timeline. False: dropping new clips in the timeline (from the project view).
    model::IClips mDraggedClips;        ///< List of all clips currently being dragged
    wxPoint mHotspot;                   ///< Hotspot within the timeline. Basically: pointer position at start of dragging.
    pts mHotspotPts;                    ///< The pts value that corresponds to the hotspot's x position. Required when changing zoom/scrolling.
    wxPoint mPosition;                  ///< Current pointer drag position. In timeline coordinates.
    wxBitmap mBitmap;                   ///< The bitmap containing the dragged clips. It is reduced to 'only visible area'. 
    wxPoint mBitmapOffset;              ///< This offset ensures that correct areas can be used when positioning on the timeline.
    bool mActive;                       ///< True if dragging is currently active.
    bool mSnap;                         ///< true if the drag image snaps to the nearest track(s)
    std::list<pts> mSnapPoints;         ///< Sorted list containing all possible 'snap to' points (pts values). Filled upon start of drag.
    std::list<pts> mDragPoints;         ///< Sorted list containing all possible 'snapping' points (pts values) in the dragged area. Filled upon start of drag.
    pts mSnapOffset;                    ///< Resulting offset caused by 'snapping to' a clip
    std::list<pts> mSnaps;              ///< List of current snapping positions (that is, where one of the dragged clips 'touches' the pts position of another clip)
    bool mMustUndo;                     ///< True if a command has been submitted for giving feedback. 
    pts mShiftPosition;                 ///< Position at which clips must be shifted to make room for the clips being dragged. ExecuteDrop::sNoShift: No shift
    pts mShiftLength;                   ///< Length of the shift required to make room for the clips being dragged. ExecuteDrop::sNoShift: No shift

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
        int mOffset;                ///< Offset by which to draw dragged tracks. Note that for 'outside' drags, it contains the id over which the new assets are being dragged.
        int mMinOffset;             ///< Min allowed value for offset (to avoid that tracks are moved into the void)
        int mMaxOffset;             ///< Mix allowed value for offset (to avoid that tracks are moved into the void)

        /// Default constructor
        DragInfo(Timeline* timeline, bool isVideo);

        ~DragInfo();

        /// Reset all offsets when a new drag operation is started
        void reset();

        /// Update the offset of tracks of this type, given a track in the timeline and a dragged track
        /// that is currently dragged above that track in the timeline.
        /// \param indexOfTrackInTimeline index of a track in the timeline
        /// \param indexOfDraggedTrack index of the track that is currently 'above' the track in the timeline
        void updateOffset(int indexOfTrackInTimeline, int indexOfDraggedTrack);

        /// \return audio or video track with given index.
        /// Whether it's an audio or a video track returned depends on the scope of this DragInfo object.
        model::TrackPtr getTrack(int index);

        /// \param track temporary track used for 'outside' drags
        void setTempTrack(model::TrackPtr track);

        /// \return temporary track used for 'outside' drags
        model::TrackPtr getTempTrack();

        model::TrackPtr trackOnTopOf(model::TrackPtr track);
        model::TrackPtr trackUnder(model::TrackPtr draggedtrack);

        int nTracks(); ///< @return number of tracks of this type currently in the timeline

    private:

        friend std::ostream& operator<< (std::ostream& os, const DragInfo& obj);

        model::TrackPtr mTempTrack; ///< For 'outside' drags, holds the temporary track that contains the new assets
        bool mIsVideo;              ///< true if this object applies to about video tracks, false if this object applies to audio tracks
        DummyView* mView;           ///< This view can be used as a container for temporary tracks used for drawing a 'outside' drag operation

    };

    DragInfo mVideo;
    DragInfo mAudio;

    model::TrackPtr mDraggedTrack;      ///< Holds, when dragging, the track directly under the mouse pointer when starting the drag (the track which is dragged)
    model::TrackPtr mDropTrack;         ///< Holds, when dragging, the track directly under the mouse pointer when dragging (the track onto which is dropped)

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
    model::TrackPtr trackOnTopOf(model::TrackPtr track);

    /// \return the track that is currently 'under' draggedtrack
    model::TrackPtr trackUnder(model::TrackPtr draggedtrack);

    /// \return the DragInfo object that corresponds to the given track.
    /// \param track track that indicates audio or video
    /// Given the track, either mVideo or mAudio is returned.
    DragInfo& getAssociatedInfo(model::TrackPtr track);

    /// Update the offset of tracks of the given track's type.
    void updateOffset(model::TrackPtr trackUnderPointer);

    /// Update the currently dragged track. The dragged track can be updated
    /// by holding SHIFT (move grab point) or by moving from audio to video
    /// or vice versa.
    /// May be called with a 0-pointer. In that case, nothing is changed.
    /// \param track track (in the timeline) on top of which the mouse pointer currently is.
    void updateDraggedTrack(model::TrackPtr track);

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

    /// Return the list of 'drops' on the given track
    /// \param track onto which clips are dropped
    /// \return list of drops onto that track
    command::ExecuteDrop::Drops getDrops(model::TrackPtr track);

    bool undo();

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<< (std::ostream& os, const Drag& obj);
    friend std::ostream& operator<< (std::ostream& os, const DragInfo& obj);
};

std::ostream& operator<< (std::ostream& os, const Drag& obj);
std::ostream& operator<< (std::ostream& os, const Drag::DragInfo& obj);

}} // namespace

#endif // DRAG_H