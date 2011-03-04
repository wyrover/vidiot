#ifndef SCROLL_H
#define SCROLL_H

#include "Part.h"
#include "UtilInt.h"

namespace gui { namespace timeline {

/// Virtual position:  position in the virtual area. That is, the entire area 
///                    that is accessible when using the scrollbars.
/// Physical position: position in coordinates of the viewable area.
class Scrolling
    :   public Part
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION METHODS
    //////////////////////////////////////////////////////////////////////////

    Scrolling(Timeline* timeline);
    virtual ~Scrolling();

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    /// Return the current scrolling offset
    wxPoint getOffset() const;

    /// Align the scroll position in such a way that the position 'sequencePts'
    /// is directly under the physical position.
    /// \param position position in the sequence
    /// \param unscrolledPixel physical position to align to
    void align(pts position, pixel unscrolledPixel);

    /// \param position position in the sequence
    /// \return physical position on the scroll window canvas
    pixel ptsToPixel(pts position) const;

    /// \return physical position
    /// \param position virtual position
    wxPoint getPhysicalPosition(wxPoint position) const;

};

}} // namespace

#endif // SCROLL_H