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
#include "Zoom.h"

namespace gui { namespace timeline {

DECLARE_EVENT(CLIP_UPDATE_EVENT, ClipUpdateEvent, ClipView*);

class Timeline;

class ClipView
    :   public wxWindow
    ,   public Part
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION METHODS
    //////////////////////////////////////////////////////////////////////////

    ClipView(
        TrackView* track,
        model::ClipPtr clip);

    void init();

	virtual ~ClipView();

    //////////////////////////////////////////////////////////////////////////
    // CONVERSION BETWEEN MODEL AND VIEW
    //////////////////////////////////////////////////////////////////////////

    model::ClipPtr getClip();

    //////////////////////////////////////////////////////////////////////////
    //  GET & SET
    //////////////////////////////////////////////////////////////////////////

    const wxBitmap& getBitmap();

    void setBeingDragged(bool beingdragged);
    bool isBeingDragged();

    /** @return left position in pixels */
    boost::int64_t getLeftPosition() const;
    /** @return right position in pixels */
    boost::int64_t getRightPosition() const;


    // tmp for showing intersect with selected regions
    void show(wxRect rect);

    void updateBitmap();

private:

    void updateSize();
    void updateThumbnail();

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
