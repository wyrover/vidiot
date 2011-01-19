#include "StateIdle.h"

#include <boost/assign/list_of.hpp>
#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>
#include "Timeline.h"
#include "StateTestDragStart.h"
#include "StateMovingCursor.h"
#include "StatePlaying.h"
#include "GuiPlayer.h"
#include "Zoom.h"
#include "Clip.h"
#include "Track.h"
#include "Sequence.h"
#include "Project.h"
#include "SplitAtCursor.h"
#include "MousePointer.h"
#include "Selection.h"
#include "StateMoveDivider.h"
#include "StateMoveTrackDivider.h"
#include "UtilLog.h"
#include "PositionInfo.h"
#include "Tooltip.h"

namespace gui { namespace timeline { namespace state {

const wxString sTooltip = _("Move the cursor to 'scrub' over the timeline and see the frames back in the preview.");

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

Idle::Idle( my_context ctx ) // entry
    :   TimeLineState( ctx )
{
    LOG_DEBUG; 
}

Idle::~Idle() // exit
{ 
    LOG_DEBUG; 
}

//////////////////////////////////////////////////////////////////////////
// EVENTS
//////////////////////////////////////////////////////////////////////////

boost::statechart::result Idle::react( const EvLeftDown& evt )
{
    VAR_DEBUG(evt);
    getWindow().SetFocus(); /** @todo make more generic, for all states */
    PointerPositionInfo info = getMousePointer().getInfo(evt.mPosition);

    if (info.onAudioVideoDivider)
    {
        return transit<MoveDivider>();
    }
    else if (info.onTrackDivider)
    {
        return transit<MoveTrackDivider>();
    }
    else
    {
        getSelection().update(info.clip,evt.mWxEvent.ControlDown(),evt.mWxEvent.ShiftDown(),evt.mWxEvent.AltDown());
        if (info.clip && !info.clip->isA<model::EmptyClip>())
        {
            return transit<TestDragStart>();
        }
        else
        {
            post_event(evt); // Handle this in the MovingCursor state.
            return transit<MovingCursor>();
        }
    }

    return discard_event();
}

boost::statechart::result Idle::react( const EvMotion& evt )
{
    VAR_DEBUG(evt);
    PointerPositionInfo info =  getMousePointer().getInfo(evt.mPosition);
    MousePointerImage image = PointerNormal;
    if (info.onAudioVideoDivider)
    {
        image = PointerTrackResize;
    }
    else
    {
        if (info.onTrackDivider)
        {
            image = PointerTrackResize;
        }
        else
        {
            if (info.clip)
            {
                switch (info.logicalclipposition)
                {
                case ClipBegin:      image = evt.mWxEvent.ShiftDown() ? PointerTrimShiftBegin : PointerTrimBegin;    break;
                case ClipBetween:    image = PointerMoveCut;     break;
                case ClipEnd:        image = evt.mWxEvent.ShiftDown() ? PointerTrimShiftEnd : PointerTrimEnd;    break;
                }
            }
        }

    }
    getMousePointer().set(image);
    return discard_event();
}

boost::statechart::result Idle::react( const EvKeyDown& evt)
{
#define WXK_NONE 0
    VAR_DEBUG(evt);
    if ( evt.mWxEvent.GetUnicodeKey() != WXK_NONE )
    {
        wxChar c = evt.mWxEvent.GetUnicodeKey();
        switch (evt.mWxEvent.GetUnicodeKey())
        {
        case 'S':   model::Project::current()->Submit(new command::SplitAtCursor(getTimeline())); break;
        }
    }
    else
    {
        switch (evt.mWxEvent.GetKeyCode())
        {
        case WXK_SPACE:     return start();                                 break;
        case WXK_DELETE:    getSelection().deleteClips();                   break;
        case WXK_F1:        getTooltip().show(sTooltip);                    break;
        }
    }
    return discard_event();
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

boost::statechart::result Idle::start()
{
getPlayer()->play();
return transit<Playing>();
}

}}} // namespace

