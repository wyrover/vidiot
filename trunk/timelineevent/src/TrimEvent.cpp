#include "TrimEvent.h"

namespace gui { namespace timeline {

DEFINE_EVENT(EVENT_TRIM_UPDATE, EventTrimUpdate, TrimEvent);

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

TrimEvent::TrimEvent(bool active, pts oldLength, pts newLength, wxString description)
    :   mActive(active)
    ,   mOldLength(oldLength)
    ,   mNewLength(newLength)
    ,   mDescription(description)
{
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

bool TrimEvent::getActive() const
{
    return mActive;
}

pts TrimEvent::getOldLength() const
{
    return mOldLength;
}

pts TrimEvent::getNewLength() const
{
    return mNewLength;
}

wxString TrimEvent::getDescription() const
{
    return mDescription;
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<( std::ostream& os, const TrimEvent& obj )
{
    os << obj.mActive << '|' << obj.mOldLength << '|' << obj.mNewLength << '|' << obj.mDescription;
    return os;
}

}} // namespace