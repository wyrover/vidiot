#ifndef DRAG_H
#define DRAG_H

#include <list>
#include <wx/bitmap.h>
#include <wx/dcmemory.h>
#include "Part.h"

namespace model {
    class Track;
    typedef boost::shared_ptr<Track> TrackPtr;
    typedef std::list<TrackPtr> Tracks;
}

namespace gui { namespace timeline {

class Drag
    :   public Part
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION METHODS
    //////////////////////////////////////////////////////////////////////////

    Drag(Timeline* timeline);
	
    //////////////////////////////////////////////////////////////////////////
    // START/STOP
    //////////////////////////////////////////////////////////////////////////

    void Start(wxPoint hotspot);
    void MoveTo(wxPoint position, bool altPressed);
    void Stop();

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////
    
    bool isActive() const; ///< @return true if a drag operation with the currently selected clips is in effect.

    //////////////////////////////////////////////////////////////////////////
    // DRAW
    //////////////////////////////////////////////////////////////////////////

    wxBitmap getDragBitmap();
    void draw(wxDC& dc) const;

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    wxPoint mHotspot;                   ///< Hotspot within the timeline. Basically: pointer position at start of dragging.
    wxPoint mPosition;                  ///< Current pointer drag position. In timeline coordinates.
    wxBitmap mBitmap;                   ///< The bitmap containing the dragged clips. It is reduced to 'only visible area'. 
    wxPoint mBitmapOffset;              ///< This offset ensures that correct areas can be used when positioning on the timeline.
    bool mActive;                       ///< True if dragging is currently active.
    bool mSnap;                         ///< true if the drag image snaps to the nearest track(s)

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
        int mOffset;    ///< Offset by which to draw dragged tracks
        int mMinOffset; ///< Min allowed value for offset (to avoid that tracks are moved into the void)
        int mMaxOffset; ///< Mix allowed value for offset (to avoid that tracks are moved into the void)

        /// Default constructor
        DragInfo(Timeline* timeline, bool isVideo);

        /// Reset all offsets when a new drag operation is started
        void reset();

        /// Update the offset of tracks of this type, given a track in the timeline and a dragged track
        /// that is currently dragged above that track in the timeline.
        /// @param indexOfTrackInTimeline index of a track in the timeline
        /// @param indexOfDraggedTrack index of the track that is currently 'above' the track in the timeline
        void updateOffset(int indexOfTrackInTimeline, int indexOfDraggedTrack);

        /// @return audio or video track with given index.
        /// Whether it's an audio or a video track returned depends on the scope of this DragInfo object.
        model::TrackPtr getTrack(int index);

        int nTracks(); ///< @return number of tracks of this type currently in the timeline

    private:

        bool mIsVideo;                  ///< true if this object applies to about video tracks, false if this object applies to audio tracks
    };

    DragInfo mVideo;
    DragInfo mAudio;

    model::TrackPtr mDraggedTrack;      ///< Holds, when dragging, the track directly under the mouse pointer when starting the drag (the track which is dragged)
    model::TrackPtr mDropTrack;         ///< Holds, when dragging, the track directly under the mouse pointer when dragging (the track onto which is dropped)

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    /// @return the track that is currently dragged on top of 'track'
    /// @return 0 if no track is found on top of this track
    /// @param track a track in the timeline
    /// Note that the returned track may be completely unselected, meaning
    /// that actually no clips from that track are visually dragged.
    model::TrackPtr trackOnTopOf(model::TrackPtr track);

    /// @return the DragInfo object that corresponds to the given track.
    /// @param track track that indicates audio or video
    /// Given the track, either mVideo or mAudio is returned.
    DragInfo& getAssociatedInfo(model::TrackPtr track);

    /// Update the currently dragged track. The dragged track can be updated
    /// by holding SHIFT (move grab point) or by moving from audio to video
    /// or vice versa.
    /// May be called with a 0-pointer. In that case, nothing is changed.
    /// @param track track (in the timeline) on top of which the mouse pointer currently is.
    void updateDraggedTrack(model::TrackPtr track);

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