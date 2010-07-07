#ifndef GUI_TIME_LINE_CLIP_H
#define GUI_TIME_LINE_CLIP_H

#include <wx/bitmap.h>
#include <wx/event.h>
#include <boost/enable_shared_from_this.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/version.hpp>
#include "AProjectViewNode.h"
#include "Clip.h"
#include "GuiPtr.h"
#include "UtilEvent.h"

DECLARE_EVENT(CLIP_UPDATE_EVENT, ClipUpdateEvent, GuiTimeLineClipPtr);

class GuiTimeLineClip
    :   public boost::enable_shared_from_this<GuiTimeLineClip>
    ,   public wxEvtHandler
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION METHODS
    //////////////////////////////////////////////////////////////////////////

    /** The default '0' pointers are used for construction in case of recovery. */
    GuiTimeLineClip(
        GuiTimeLineZoomPtr zoom = GuiTimeLineZoomPtr(), 
        model::ClipPtr clip = model::ClipPtr());
    /**
     * Two step construction. First the constructor (in combination with serialize)
     * sets all relevant  members. Second, this method initializes all GUI stuff 
     * including the bitmap. 
     * @param track track to which this clip belongs
     * @param allclips list of all clips in the timeline of this clip (used for linking clips)
     */
    void init(boost::weak_ptr<GuiTimeLineTrack> track, GuiTimeLineClips& allclips);
	virtual ~GuiTimeLineClip();

    const wxBitmap& getBitmap();

    bool isSelected() const;
    void setSelected(bool selected);
    void setBeingDragged(bool beingdragged);
    bool isBeingDragged();

    GuiTimeLineTrackPtr getTrack() const;
    
    model::ClipPtr getClip() const;
    GuiTimeLineClipPtr GuiTimeLineClip::getLink() const;

private:
    GuiTimeLineZoomPtr mZoom;
    boost::weak_ptr<GuiTimeLineTrack> mTrack;

    void updateSize();
    void updateThumbnail();
    void updateBitmap();

    bool mSelected;
    bool mBeingDragged;
    int mWidth;
    boost::weak_ptr<GuiTimeLineClip> mLink; /** /todo destruction: these bidi links will keep both ends alive... */
    model::ClipPtr mClip;
    boost::scoped_ptr<wxBitmap> mThumbnail;
    wxBitmap mBitmap;

    //////////////////////////////////////////////////////////////////////////
    // SERIALIZATION 
    //////////////////////////////////////////////////////////////////////////

    friend class boost::serialization::access;
    template<class Archive> 
    void serialize(Archive & ar, const unsigned int version);
};

BOOST_CLASS_VERSION(GuiTimeLineClip, 1)
BOOST_CLASS_EXPORT(GuiTimeLineClip)

#endif // GUI_TIME_LINE_CLIP_H