#include "Tooltip.h"

#include <wx/tooltip.h>
#include "Timeline.h"
#include "UtilLog.h"

namespace gui { namespace timeline {

Tooltip::Tooltip(Timeline* timeline)
:   Part(timeline)
{
    wxToolTip::SetDelay(0);
}

//////////////////////////////////////////////////////////////////////////
// DRAW
//////////////////////////////////////////////////////////////////////////

void Tooltip::show( const wxString& text )
{
    VAR_INFO(text);
    if (getTimeline().GetToolTipText().IsSameAs(text) || text.IsSameAs(_("")))
    {
        getTimeline().UnsetToolTip();
    }
    else
    {
        getTimeline().SetToolTip(text);
    }
}

}} // namespace
