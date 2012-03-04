#ifndef INTERVALS_VIEW_H
#define INTERVALS_VIEW_H

#include "View.h"

namespace gui { namespace timeline {

class IntervalsView
    :   public View
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION METHODS
    //////////////////////////////////////////////////////////////////////////

    IntervalsView(View* parent);
    virtual ~IntervalsView();

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    wxSize requiredSize() const override;  ///< @see View::requiredSize()

    //////////////////////////////////////////////////////////////////////////
    // DRAWING
    //////////////////////////////////////////////////////////////////////////

    /// This view draws onto the bitmap of the parent view, and not onto its
    /// own bitmap. Rationale: the intervals are depicted 'on top' of the sequence,
    /// and are not drawn onto a separate region in the timeline.
    void draw(wxDC& dc) const;

    void refreshInterval(PtsInterval interval);

private:

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    wxRect makeRect(PtsInterval interval) const;
    void draw(wxBitmap& bitmap) const override; ///< @see View::draw()
};

}} // namespace

#endif // INTERVALS_VIEW_H