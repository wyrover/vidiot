#ifndef GUI_TIME_LINE_TRACK_H
#define GUI_TIME_LINE_TRACK_H

#include <wx/bitmap.h>
#include <wx/event.h>
#include <wx/dcmemory.h>
#include <wx/window.h>
#include <boost/tuple/tuple.hpp>
#include <boost/optional.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/version.hpp>
#include "ModelPtr.h"
#include "GuiPtr.h"
#include "Track.h"
#include "UtilEvent.h"
#include "GuiTimeLineZoom.h"
#include "ViewMap.h"

namespace gui { namespace timeline {

class ClipUpdateEvent;

DECLARE_EVENT(TRACK_UPDATE_EVENT, TrackUpdateEvent, GuiTimeLineTrackPtr);

class GuiTimeLineTrack
    :   public wxWindow
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION METHODS
    //////////////////////////////////////////////////////////////////////////

    /** Recovery constructor equals two '0' pointers. */
    GuiTimeLineTrack(
        GuiTimeLine* timeline,
        const GuiTimeLineZoom& zoom, 
        ViewMap& viewMap, 
        model::TrackPtr track);

    /**
     * Two step construction. First the constructor (in combination with serialize)
     * sets all relevant  members. Second, this method initializes all GUI stuff
     * including the bitmap. This is also used to facilitate using shared_from_this()
     * during construction (as using this in a constructor can lead to problems).
     * @param timeline timeline to which this track belongs.
     */
    void init(GuiTimeLine* timeline);

	virtual ~GuiTimeLineTrack();

    int getClipHeight() const;

    model::TrackPtr getTrack() const;


    // todo hide this as the getheight should not be used on the bitmap but on the track
    // for initialization purposes.
    const wxBitmap& getBitmap();

    void drawClips(wxPoint position, wxMemoryDC& dc, boost::optional<wxMemoryDC&> dcSelectedClipsMask = boost::none) const;

    //////////////////////////////////////////////////////////////////////////
    // GUI EVENTS
    //////////////////////////////////////////////////////////////////////////

    void OnIdle(wxIdleEvent& event);

    //////////////////////////////////////////////////////////////////////////
    // MODEL EVENTS
    //////////////////////////////////////////////////////////////////////////

    void OnClipsAdded( model::EventAddClips& event );
    void OnClipsRemoved( model::EventRemoveClips& event );

    //////////////////////////////////////////////////////////////////////////
    // DRAWING EVENTS
    //////////////////////////////////////////////////////////////////////////

    void OnClipUpdated( ClipUpdateEvent& event );

private:
    friend class SelectIntervals;

    const GuiTimeLineZoom& mZoom;
    ViewMap& mViewMap;

    GuiTimeLine* mTimeLine;

    void updateBitmap();

    model::TrackPtr mTrack;
    wxBitmap mBitmap;

    bool mRedrawOnIdle;
};

}} // namespace

#endif // GUI_TIME_LINE_TRACK_H
