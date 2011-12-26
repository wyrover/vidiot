#ifndef THUMBNAIL_VIEW_H
#define THUMBNAIL_VIEW_H

#include <boost/shared_ptr.hpp>
#include "View.h"

namespace model{
class IClip;
typedef boost::shared_ptr<IClip> IClipPtr;
class VideoClip;
typedef boost::shared_ptr<VideoClip> VideoClipPtr;
}

namespace gui { namespace timeline {

class ThumbnailView
    :   public View
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION METHODS
    //////////////////////////////////////////////////////////////////////////

    ThumbnailView(model::IClipPtr clip, View* parent);
    virtual ~ThumbnailView();

    //////////////////////////////////////////////////////////////////////////
    //  GET & SET
    //////////////////////////////////////////////////////////////////////////

    wxSize requiredSize() const override;    ///< \see View::requiredSize()

    void redraw(); ///< Invalidates the bitmap

private:

    model::VideoClipPtr mVideoClip;

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    void draw(wxBitmap& bitmap) const override;                      ///< @see View::draw()
};

}} // namespace

#endif // THUMBNAIL_VIEW_H