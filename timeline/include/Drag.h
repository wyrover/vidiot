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
    
	virtual wxRect GetImageRect(const wxPoint& pos) const;
    wxBitmap getDragBitmap();
    bool isActive() const; ///< @return true if a drag operation with the currently selected clips is in effect.

    //////////////////////////////////////////////////////////////////////////
    // DRAW
    //////////////////////////////////////////////////////////////////////////

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

    struct DragInfo
    {
        int mOffset;    ///< Offset by which to draw dragged tracks
        int mMinOffset; ///< Min allowed value for offset (to avoid that tracks are moved into the void)
        int mMaxOffset; ///< Mix allowed value for offset (to avoid that tracks are moved into the void)
        int nTracks;    ///< Total number of tracks of this type
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

        //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<< (std::ostream& os, const Drag& obj);
};

std::ostream& operator<< (std::ostream& os, const Drag& obj);

}} // namespace

#endif // DRAG_H