#ifndef CLIP_VIEW_H
#define CLIP_VIEW_H

#include <wx/bitmap.h>
#include <wx/event.h>
#include <wx/window.h>
#include <boost/scoped_ptr.hpp>
#include "AProjectViewNode.h"
#include "Clip.h"
#include "GuiPtr.h"
#include "UtilEvent.h"
#include "Part.h"

namespace gui { namespace timeline {

DECLARE_EVENT(CLIP_UPDATE_EVENT, ClipUpdateEvent, ClipView*);

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

#endif // CLIP_VIEW_H
