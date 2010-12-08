#ifndef CLIP_VIEW_H
#define CLIP_VIEW_H

#include <boost/scoped_ptr.hpp>
#include "View.h"

namespace model{
class Clip;
typedef boost::shared_ptr<Clip> ClipPtr;
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

    ClipView(model::ClipPtr clip, View* parent);
    virtual ~ClipView();

    //////////////////////////////////////////////////////////////////////////
    //  GET & SET
    //////////////////////////////////////////////////////////////////////////

    model::ClipPtr getClip();

    pixel getLeftPosition() const;  ///< @return left position in pixels
    pixel getRightPosition() const; ///< @return right position in pixels

    pixel requiredWidth() const;  ///< @see View::requiredWidth()
    pixel requiredHeight() const; ///< @see View::requiredHeight()

    void getPositionInfo(wxPoint position, PointerPositionInfo& info) const;

    void show(wxRect rect); ///< tmp for showing intersect with selected regions

private:

    model::ClipPtr mClip;
    boost::scoped_ptr<wxBitmap> mThumbnail;

    wxRect mRect; ///< @see show()

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    void updateThumbnail();

    void draw(wxBitmap& bitmap) const; ///< @see View::draw()

};

}} // namespace

#endif // CLIP_VIEW_H
