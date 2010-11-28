#ifndef CLIP_VIEW_H
#define CLIP_VIEW_H

#include <boost/scoped_ptr.hpp>
#include <boost/cstdint.hpp>
#include "View.h"

namespace gui { namespace timeline {

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
    // CONVERSION BETWEEN MODEL AND VIEW
    //////////////////////////////////////////////////////////////////////////

    model::ClipPtr getClip();

    //////////////////////////////////////////////////////////////////////////
    //  GET & SET
    //////////////////////////////////////////////////////////////////////////

    /** @return left position in pixels */
    boost::int64_t getLeftPosition() const;
    /** @return right position in pixels */
    boost::int64_t getRightPosition() const;

    /** @see View::requiredWidth() **/
    int requiredWidth();

    /** @see View::requiredHeight() **/
    int requiredHeight();

    // tmp for showing intersect with selected regions
    void show(wxRect rect);

private:

    void updateThumbnail();

    model::ClipPtr mClip;
    boost::scoped_ptr<wxBitmap> mThumbnail;

    // tmp for showing intersect with selected regions
    wxRect mRect;
    
    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    /** @see View::draw() */
    void draw(wxBitmap& bitmap);

};

}} // namespace

#endif // CLIP_VIEW_H
