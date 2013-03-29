#include "StateIdle.h"

#include "AudioTransitionFactory.h"
#include "Clip.h"
#include "ClipView.h"
#include "CreateTransition.h"
#include "Drag.h"
#include "EmptyClip.h"
#include "EventDrag.h"
#include "EventKey.h"
#include "EventMouse.h"
#include "EventPart.h"
#include "Menu.h"
#include "MousePointer.h"
#include "Player.h"
#include "PositionInfo.h"
#include "Selection.h"
#include "Sequence.h"
#include "SplitAtCursor.h"
#include "StateDragging.h"
#include "StateLeftDown.h"
#include "StateMoveDivider.h"
#include "StateMoveTrackDivider.h"
#include "StateMovingCursor.h"
#include "StatePlaying.h"
#include "StateRightDown.h"
#include "StateScrolling.h"
#include "StateTrim.h"
#include "Timeline.h"
#include "Tooltip.h"
#include "Track.h"
#include "UtilLog.h"
#include "VideoTrack.h"
#include "VideoTransitionFactory.h"
#include "ViewMap.h"
#include "Zoom.h"

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
    return leftDown();
}

boost::statechart::result Idle::react( const EvLeftDouble& evt )
{
    VAR_DEBUG(evt);
    return leftDown();
 }

boost::statechart::result Idle::react( const EvRightDown& evt )
{
    VAR_DEBUG(evt);
    PointerPositionInfo info = getMousePointer().getInfo(evt.mPosition);

    if (info.onAudioVideoDivider)
    {
        // Keep selection intact
    }
    else if (info.onTrackDivider)
    {
        // Keep selection intact
    }
    else
    {
        getSelection().updateOnRightClick(info.clip);
    }
    return transit<StateRightDown>();
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
                case TransitionBegin:
                case TransitionRightClipBegin: // FALLTHROUGH
                case ClipBegin:      image = evt.mWxEvent.ShiftDown() ? PointerTrimShiftBegin : PointerTrimBegin;    break;
                case TransitionEnd:
                case TransitionLeftClipEnd: // FALLTHROUGH
                case ClipEnd:        image = evt.mWxEvent.ShiftDown() ? PointerTrimShiftEnd : PointerTrimEnd;    break;
                }
            }
        }

    }
    getMousePointer().set(image);
    return forward_event();
}

boost::statechart::result Idle::react( const EvKeyDown& evt)
{
    VAR_DEBUG(evt);
    if ( evt.mWxEvent.GetUnicodeKey() != WXK_NONE )
    {
        wxChar c = evt.mWxEvent.GetUnicodeKey();
        switch (evt.mWxEvent.GetUnicodeKey())
        {
        case 's':   (new command::SplitAtCursor(getSequence()))->submit(); break;
        case 'S':   (new command::SplitAtCursor(getSequence()))->submit(); break;
        case 'c':   addTransition(); break;
        case 'C':   addTransition(); break;
        case '-':   getZoom().change( evt.mWxEvent.ControlDown() ? -1000 : -1); break;
        case '=':   getZoom().change( evt.mWxEvent.ControlDown() ?  1000 :  1); break;
        case ' ':     return start();                                 break;
        }
    }
    else
    {
        switch (evt.mWxEvent.GetKeyCode())
        {
        case WXK_SPACE:     return start();                                 break;
        case WXK_DELETE:    getSelection().deleteClips();                   break;
        case WXK_F1:        getTooltip().show(sTooltip);                    break;
        case 'c': addTransition(); break;
        case 'C': addTransition(); break;
        case 's':   (new command::SplitAtCursor(getSequence()))->submit(); break;
        case 'S':   (new command::SplitAtCursor(getSequence()))->submit(); break;
        case '-':   getZoom().change( evt.mWxEvent.ControlDown() ? -1000 : -1); break;
        case '=':   getZoom().change( evt.mWxEvent.ControlDown() ?  1000 :  1); break;
        }
    }
    return forward_event();
}

boost::statechart::result Idle::react( const EvDragEnter& evt)
{
    getDrag().start(evt.mPosition, false);
    return transit<Dragging>();
}

boost::statechart::result Idle::react( const EvPlaybackChanged& evt)
{
    if (evt.mActive)
    {
        return transit<Playing>();
    }
    return discard_event();
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

boost::statechart::result Idle::start()
{
    getPlayer()->play();
    return discard_event();
}

boost::statechart::result Idle::leftDown()
{
    PointerPositionInfo info = getMousePointer().getInfo(getMousePointer().getLeftDownPosition());

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
        getSelection().updateOnLeftClick(info);
        if (info.clip && !info.clip->isA<model::EmptyClip>())
        {
            switch (info.logicalclipposition)
            {
            case ClipBegin:
                return transit<StateTrim>();
                break;
            case ClipInterior:
            case TransitionLeftClipInterior:
            case TransitionRightClipInterior:
                return transit<StateLeftDown>();
                break;
            case ClipEnd:
            case TransitionLeftClipEnd:
            case TransitionRightClipBegin:
                return transit<StateTrim>();
                break;
            case TransitionBegin:
            case TransitionEnd:
                return transit<StateTrim>();
            case TransitionInterior:
                return forward_event();
            default:
                FATAL("Unexpected logical clip position.");
            }
        }
        else
        {
            return transit<MovingCursor>();
        }
    }

    return forward_event();
}

void Idle::addTransition()
{
    PointerPositionInfo info = getTimeline().getMousePointer().getInfo(getMousePointer().getPosition());
    if (info.clip)
    {
        model::TransitionType type;
        switch (info.logicalclipposition)
        {
        case ClipBegin: type = model::TransitionTypeInOut; break;
        case ClipEnd:   type = model::TransitionTypeOutIn; break;
        default:        return; // Do nothing: No transition is created.
        }

        ASSERT(info.track);
        model::TransitionPtr transition = info.track->isA<model::VideoTrack>() ? model::video::VideoTransitionFactory::get().getDefault() : model::audio::AudioTransitionFactory::get().getDefault();
        command::CreateTransition* cmd = new command::CreateTransition(getSequence(), info.clip, transition, type);
        if (cmd->isPossible())
        {
            cmd->submit();
        }
        else
        {
            delete cmd;
        }
    }
}

}}} // namespace