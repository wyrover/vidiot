#include "SplitAtCursor.h"

#include <boost/foreach.hpp>
#include "UtilLog.h"
#include "Timeline.h"
#include "Cursor.h"
#include "Track.h"
#include "Sequence.h"
#include "Zoom.h"

namespace gui { namespace timeline { namespace command {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

SplitAtCursor::SplitAtCursor(gui::timeline::Timeline& timeline)
    :   AClipEdit(timeline)
{
    VAR_INFO(this);
    mCommandName = _("Insert split at cursor position");
}

SplitAtCursor::~SplitAtCursor()
{
}

//////////////////////////////////////////////////////////////////////////
// ACLIPEDIT INTERFACE
//////////////////////////////////////////////////////////////////////////

void SplitAtCursor::initialize()
{
    VAR_INFO(this);
    pts position = getTimeline().getZoom().pixelsToPts(getTimeline().getCursor().getPosition());
    splittrack(getTimeline().getSequence()->getVideoTracks(), position );
    splittrack(getTimeline().getSequence()->getAudioTracks(), position );
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void SplitAtCursor::splittrack(model::Tracks tracks, pts position)
{
    BOOST_FOREACH( model::TrackPtr track, tracks )
    {
        split(track, position);
    }
}

}}} // namespace
