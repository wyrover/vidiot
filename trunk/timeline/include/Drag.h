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
    void MoveTo(wxPoint position);
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

    int mVideoTrackOffset;              ///< Offset by which to draw dragged video tracks
    int mAudioTrackOffset;              ///< Offset by which to draw dragged audio tracks

    model::TrackPtr mFirstVideoTrack;   ///< Holds, when dragging, the first (bottom) dragged video track
    model::TrackPtr mLastVideoTrack;    ///< Holds, when dragging, the last  (top)    dragged video track
    model::TrackPtr mFirstAudioTrack;   ///< Holds, when dragging, the first (top)    dragged video track
    model::TrackPtr mLastAudioTrack;    ///< Holds, when dragging, the last  (bottom) dragged video track

    model::TrackPtr mDraggedTrack;      ///< Holds, when dragging, the track directly under the mouse pointer when starting the drag (the track which is dragged)
    model::TrackPtr mDropTrack;         ///< Holds, when dragging, the track directly under the mouse pointer when dragging (the track onto which is dropped)

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<< (std::ostream& os, const Drag& obj);
};

std::ostream& operator<< (std::ostream& os, const Drag& obj);

}} // namespace

#endif // DRAG_H