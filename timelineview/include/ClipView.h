#ifndef CLIP_VIEW_H
#define CLIP_VIEW_H

#include <boost/scoped_ptr.hpp>
#include "View.h"

namespace model{
class IClip;
typedef boost::shared_ptr<IClip> IClipPtr;
class EventSelectClip;
class DebugEventRenderProgress;
}

namespace gui { namespace timeline {
    struct PointerPositionInfo;

class ClipView
    :   public View
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION METHODS
    //////////////////////////////////////////////////////////////////////////

    ClipView(model::IClipPtr clip, View* parent);
    virtual ~ClipView();

    //////////////////////////////////////////////////////////////////////////
    //  GET & SET
    //////////////////////////////////////////////////////////////////////////

    model::IClipPtr getClip();

    pixel getLeftPosition() const;  ///< @return left position in pixels
    pixel getRightPosition() const; ///< @return right position in pixels

    pixel requiredWidth() const;  ///< @see View::requiredWidth()
    pixel requiredHeight() const; ///< @see View::requiredHeight()

    void getPositionInfo(wxPoint position, PointerPositionInfo& info) const;

    void show(wxRect rect); ///< tmp for showing intersect with selected regions

    /// This will reread the thumbnail from disk. It will also invalidate the 
    /// bitmap and force a redraw.
    void updateThumbnail(bool invalidate = false);

    //////////////////////////////////////////////////////////////////////////
    // DRAW
    //////////////////////////////////////////////////////////////////////////

    void drawForDragging(wxPoint position, int height, wxDC& dc, wxDC& dcMask) const;

    //////////////////////////////////////////////////////////////////////////
    // MODEL EVENTS
    //////////////////////////////////////////////////////////////////////////

    void onClipSelected( model::EventSelectClip& event );
    void onGenerationProgress( model::DebugEventRenderProgress& event );

private:

    model::IClipPtr mClip;
    boost::scoped_ptr<wxBitmap> mThumbnail;

    wxRect mRect;       ///< @see show()
    pts mBeginAddition; ///< if >0 then this area is (temporarily, during editing) added. if <0 then it is removed from the clip.

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    void draw(wxBitmap& bitmap) const;                      ///< @see View::draw()

    /// Actual drawing implementation. Reused for drawing clips when dragging.
    /// \param drawDraggedClips indicates if clips currently being dragged must be drawn or left empty
    /// \param drawNotDraggedClips indicates if clips currently NOT being dragged  must be drawn or left empty
    void draw(wxBitmap& bitmap, bool drawDraggedClips, bool drawNotDraggedClips) const;

};

}} // namespace

#endif // CLIP_VIEW_H
