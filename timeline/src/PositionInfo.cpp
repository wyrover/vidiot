#include "PositionInfo.h"

#include "Config.h"

namespace gui { namespace timeline {

IMPLEMENTENUM(MouseOnClipPosition);

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

PointerPositionInfo::PointerPositionInfo()
:   onAudioVideoDivider(false)
,   track(model::TrackPtr())
,   trackPosition(0)
,   onTrackDivider(false)
,   clip(model::IClipPtr())
,   logicalclipposition(ClipInterior)
{
}

PointerPositionInfo::~PointerPositionInfo()
{
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<( std::ostream& os, const PointerPositionInfo& obj )
{
    os << &obj << '|' << obj.onAudioVideoDivider << '|' << obj.track << '|' << obj.trackPosition << '|' << obj.onTrackDivider << '|' << obj.clip << '|' << obj.logicalclipposition;
    return os;
}

}} // namespace