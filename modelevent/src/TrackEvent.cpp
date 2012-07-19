#include "TrackEvent.h"

#include "UtilLogStl.h"
#include "IClip.h"

namespace model {

DEFINE_EVENT(EVENT_ADD_CLIPS,           EventAddClips,          MoveParameter);
DEFINE_EVENT(EVENT_REMOVE_CLIPS,        EventRemoveClips,       MoveParameter);
DEFINE_EVENT(EVENT_HEIGHT_CHANGED,      EventHeightChanged,     int);

MoveParameter::MoveParameter()
:   addTrack()
,   addPosition()
,   addClips()
,   removeTrack()
,   removePosition()
,   removeClips()
{
}

MoveParameter::MoveParameter(TrackPtr _addTrack, IClipPtr _addPosition, IClips _addClips, TrackPtr _removeTrack, IClipPtr _removePosition, IClips _removeClips)
:   addTrack(_addTrack)
,   addPosition(_addPosition)
,   addClips(_addClips)
,   removeTrack(_removeTrack)
,   removePosition(_removePosition)
,   removeClips(_removeClips)
{
}

MoveParameter::MoveParameter(const MoveParameter& other)
:   addTrack(other.addTrack)
,   addPosition(other.addPosition)
,   addClips(other.addClips)
,   removeTrack(other.removeTrack)
,   removePosition(other.removePosition)
,   removeClips(other.removeClips)
{
}

MoveParameterPtr MoveParameter::make_inverted()
{
    return boost::make_shared<MoveParameter>(removeTrack,removePosition,removeClips,addTrack,addPosition,addClips);
}

std::ostream& operator<<( std::ostream& os, const MoveParameter& obj )
{
    os << obj.removeTrack << '|' << obj.removePosition << '|' << obj.removeClips << '|' << obj.addTrack << '|' << obj.addPosition << '|' << obj.addClips;
    return os;
}

} // namespace