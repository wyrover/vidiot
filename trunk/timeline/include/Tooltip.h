#ifndef TOOLTIP_H
#define TOOLTIP_H

#include "Part.h"

namespace gui { namespace timeline {

class Tooltip
    :   public Part
{
public:
    Tooltip(Timeline* timeline);
    virtual ~Tooltip();

    //////////////////////////////////////////////////////////////////////////
    // DRAW
    //////////////////////////////////////////////////////////////////////////

    /// Shows or hides the current tooltip.
    /// If the given text equals the current tooltip -> hide
    /// If the given text equals "" -> hide
    /// In all other cases -> show new tooltip containing text
    /// @param tooltip to be shown or hidden
    void show( const wxString& text );
};

}} // namespace

#endif // TOOLTIP_H
