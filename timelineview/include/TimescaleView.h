#ifndef TIMESCALE_VIEW_H
#define TIMESCALE_VIEW_H

#include "View.h"

namespace gui { namespace timeline {

class TimescaleView
    :   public View
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION METHODS
    //////////////////////////////////////////////////////////////////////////

    TimescaleView(View* parent);
    virtual ~TimescaleView();

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    wxSize requiredSize() const override;  ///< @see View::requiredSize()

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    void draw(wxBitmap& bitmap) const override; ///< @see View::draw()
};

}} // namespace

#endif // TIMESCALE_VIEW_H