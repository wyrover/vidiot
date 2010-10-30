#ifndef GUI_TIME_LINE_TRACK_H
#define GUI_TIME_LINE_TRACK_H

#include <wx/bitmap.h>
#include <wx/event.h>
#include <wx/dcmemory.h>
#include <boost/tuple/tuple.hpp>
#include <boost/optional.hpp>
#include <boost/enable_shared_from_this.hpp>
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
    :   public boost::enable_shared_from_this<GuiTimeLineTrack>
    ,   public wxEvtHandler
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION METHODS
    //////////////////////////////////////////////////////////////////////////

    /** Recovery constructor equals two '0' pointers. */
    GuiTimeLineTrack(
        const GuiTimeLineZoom& zoom, 
        ViewMap& viewMap, 
        model::TrackPtr track);

    /**
     * Two step construction. First the constructor (in combination with serialize)
     * sets all relevant  members. Second, this method initializes all GUI stuff
     * including the bitmap.
     * @param timeline timeline to which this track belongs
     */
    void init(GuiTimeLine* timeline);

	virtual ~GuiTimeLineTrack();

    int getClipHeight() const;

    int getIndex();

    model::TrackPtr getTrack() const;


    // todo hide this as the getheight should not be used on the bitmap but on the track
    // for initialization purposes.
    const wxBitmap& getBitmap();

    GuiTimeLineClips getClips() const;

    void drawClips(wxPoint position, wxMemoryDC& dc, boost::optional<wxMemoryDC&> dcSelectedClipsMask = boost::none) const;

    /**
     * @return found clip and its leftmost position within the track
     * @return null pointer and 0 if not found
     */
    GuiTimeLineClipWithOffset findClip(int position);
    boost::tuple<int,int> findClipBounds(GuiTimeLineClipPtr findclip);

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

    int mHeight;
    GuiTimeLineClips mClips;
    model::TrackPtr mTrack;
    wxBitmap mBitmap;
};

}} // namespace

#endif // GUI_TIME_LINE_TRACK_H
