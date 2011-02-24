#ifndef SCROLL_H
#define SCROLL_H

#include "Part.h"
#include "UtilInt.h"

namespace gui { namespace timeline {

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

    /// Align the scroll position in such a way that the position 'sequence'
    /// is directly under the current pointer position.
    /// \param sequence position in the sequence
    /// \param pointer (unscrolled) position of the mouse pointer
    void align(pts sequence, pts pointer);
};

}} // namespace

#endif // SCROLL_H