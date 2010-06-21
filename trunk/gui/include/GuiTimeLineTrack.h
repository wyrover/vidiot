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
#include "ClipUpdateEvent.h"

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
        GuiTimeLineZoomPtr zoom = GuiTimeLineZoomPtr(), 
        model::TrackPtr track = model::TrackPtr());

    /**
     * Two step construction. First the constructor (in combination with serialize)
     * sets all relevant  members. Second, this method initializes all GUI stuff 
     * including the bitmap.
     * @param timeline timeline to which this track belongs
     * @param allclips list of all clips in this timeline (used for linking clips together)
     */
    void init(GuiTimeLine* timeline, GuiTimeLineClips& allclips);
	virtual ~GuiTimeLineTrack();

    int getClipHeight() const;

    // todo hide this as the getheight should not be used on the bitmap but on the track
    // for initialization purposes.
    const wxBitmap& getBitmap();

    GuiTimeLineClips getClips() const;

    void drawClips(wxPoint position, wxMemoryDC& dc, boost::optional<wxMemoryDC&> dcSelectedClipsMask = boost::none) const;

    GuiTimeLineClipPtr findClip(int position);
    boost::tuple<int,int> findClipBounds(GuiTimeLineClipPtr findclip);

    //////////////////////////////////////////////////////////////////////////
    // DRAWING EVENTS
    //////////////////////////////////////////////////////////////////////////

    void OnClipUpdated( ClipUpdateEvent& event );

private:
    GuiTimeLineZoomPtr mZoom;
    GuiTimeLine* mTimeLine;

    void updateBitmap();

    int mHeight;
    GuiTimeLineClips mClips;
    model::TrackPtr mTrack;
    wxBitmap mBitmap;

    //////////////////////////////////////////////////////////////////////////
    // SERIALIZATION 
    //////////////////////////////////////////////////////////////////////////

    friend class boost::serialization::access;
    template<class Archive> 
    void serialize(Archive & ar, const unsigned int version);
};

BOOST_CLASS_VERSION(GuiTimeLineTrack, 1)
BOOST_CLASS_EXPORT(GuiTimeLineTrack)

#endif // GUI_TIME_LINE_TRACK_H