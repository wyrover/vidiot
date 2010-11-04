#ifndef GUI_TIME_LINE_CLIP_H
#define GUI_TIME_LINE_CLIP_H

#include <wx/bitmap.h>
#include <wx/event.h>
#include <wx/window.h>
#include <boost/scoped_ptr.hpp>
#include "AProjectViewNode.h"
#include "Clip.h"
#include "GuiPtr.h"
#include "UtilEvent.h"
#include "GuiTimeLineZoom.h"
#include "ViewMap.h"

namespace gui { namespace timeline {

DECLARE_EVENT(CLIP_UPDATE_EVENT, ClipUpdateEvent, GuiTimeLineClipPtr);

class GuiTimeLineClip
    :   public wxWindow
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION METHODS
    //////////////////////////////////////////////////////////////////////////

    GuiTimeLineClip(
        GuiTimeLineTrack* track,
        const GuiTimeLineZoom& zoom, 
        ViewMap& viewMap, 
        model::ClipPtr clip);

	virtual ~GuiTimeLineClip();

    //////////////////////////////////////////////////////////////////////////
    // CONVERSION BETWEEN MODEL AND VIEW
    //////////////////////////////////////////////////////////////////////////

    model::ClipPtr getClip();
    GuiTimeLineTrackPtr getTrack();

    //////////////////////////////////////////////////////////////////////////
    //  GET & SET
    //////////////////////////////////////////////////////////////////////////

    const wxBitmap& getBitmap();

    bool isEmpty() const;
    bool isSelected() const;
    void setSelected(bool selected);
    void setBeingDragged(bool beingdragged);
    bool isBeingDragged();

    /** @return left position in pixels */
    boost::int64_t getLeftPosition() const;
    /** @return right position in pixels */
    boost::int64_t getRightPosition() const;


    // tmp for showing intersect with selected regions
    void show(wxRect rect);

private:

    const GuiTimeLineZoom& mZoom;
    ViewMap& mViewMap;

    void updateSize();
    void updateThumbnail();
    void updateBitmap();

    bool mSelected;
    bool mBeingDragged;
    int mWidth;
    model::ClipPtr mClip;
    boost::scoped_ptr<wxBitmap> mThumbnail;
    wxBitmap mBitmap;

    // tmp for showing intersect with selected regions
    wxRect mRect;
};

}} // namespace

#endif // GUI_TIME_LINE_CLIP_H
