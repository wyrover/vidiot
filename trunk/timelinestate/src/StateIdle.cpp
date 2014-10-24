// Copyright 2013,2014 Eric Raijmakers.
//
// This file is part of Vidiot.
//
// Vidiot is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Vidiot is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Vidiot. If not, see <http://www.gnu.org/licenses/>.

#include "StateIdle.h"

#include "AudioTransitionFactory.h"
#include "Clip.h"
#include "ClipView.h"
#include "CreateTransitionHelper.h"
#include "Cursor.h"
#include "Drag.h"
#include "EmptyClip.h"
#include "EventDrag.h"
#include "EventKey.h"
#include "EventMouse.h"
#include "EventPart.h"
#include "Menu.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "Player.h"
#include "PositionInfo.h"
#include "ProjectModification.h"
#include "Selection.h"
#include "Sequence.h"
#include "SplitAtCursor.h"
#include "SplitAtCursorAndTrim.h"
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

const wxString Idle::sTooltip = _(
    "Move the cursor to 'scrub' over the timeline and see the frames back in the preview."
    );

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

Idle::Idle( my_context ctx ) // entry
    :   TimeLineState( ctx )
{
    LOG_DEBUG;
    updateMouseCursor();
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
    return rightDown();
}

boost::statechart::result Idle::react( const EvRightDouble& evt )
{
    VAR_DEBUG(evt);
    return rightDown();
}

boost::statechart::result Idle::react( const EvMotion& evt )
{
    VAR_DEBUG(evt);
    updateMouseCursor();
    return forward_event();
}

boost::statechart::result Idle::react( const EvKeyDown& evt)
{
    VAR_DEBUG(evt);
    switch (evt.KeyCode)
    {
    case WXK_SPACE:
        evt.consumed();
        return start();
        break;
    case WXK_DELETE:
        evt.consumed();
        getSelection().deleteClips(getKeyboard().getShiftDown());
        break;
    case WXK_F1:
        getTooltip().show(sTooltip);
        break;
    case 'b':
    case 'B':
        evt.consumed();
        model::ProjectModification::submitIfPossible(new command::SplitAtCursorAndTrim(getSequence(), true));
        break;
    case 'e':
    case 'E':
        evt.consumed();
        model::ProjectModification::submitIfPossible(new command::SplitAtCursorAndTrim(getSequence(), false));
        break;
    case 'c':
    case 'C':
        evt.consumed();
        addTransition(model::TransitionTypeInOut);
        break;
    case 'i':
    case 'I':
        evt.consumed();
        addTransition(model::TransitionTypeIn);
        break;
    case 'n':
    case 'N':
        evt.consumed();
        addTransition(model::TransitionTypeOutIn);
        break;
    case 'o':
    case 'O':
        evt.consumed();
        addTransition(model::TransitionTypeOut);
        break;
    case 'p':
    case 'P':
        evt.consumed();
        addTransition(model::TransitionTypeInOut);
        break;
    case 's':
    case 'S':
        evt.consumed();
        model::ProjectModification::submitIfPossible(new command::SplitAtCursor(getSequence()));
        break;
    case '-':
        evt.consumed();
        getZoom().change( evt.CtrlDown ? -1000 : -1);
        break;
    case '=':
        evt.consumed();
        getZoom().change( evt.CtrlDown ?  1000 :  1);
        break;
    case 'v':
    case 'V':
        evt.consumed();
        getCursor().center();
        break;
    case WXK_LEFT:
        evt.consumed();
        evt.CtrlDown ? getCursor().prevCut() : getCursor().prevFrame();
        break;
    case WXK_RIGHT:
        evt.consumed();
        evt.CtrlDown ? getCursor().nextCut() : getCursor().nextFrame();
        break;
    case WXK_HOME:
        evt.consumed();
        getCursor().home();
        break;
    case WXK_END:
        evt.consumed();
        getCursor().end();
        break;
    }
    return forward_event();
}

boost::statechart::result Idle::react( const EvDragEnter& evt)
{
    getDrag().start(getMouse().getVirtualPosition(), true);
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

void Idle::updateMouseCursor()
{
    PointerPositionInfo info =  getMouse().getInfo(getMouse().getVirtualPosition());
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
                case ClipBegin:      image = getKeyboard().getShiftDown() ? PointerTrimShiftBegin : PointerTrimBegin;    break;
                case TransitionEnd:
                case TransitionLeftClipEnd: // FALLTHROUGH
                case ClipEnd:        image = getKeyboard().getShiftDown() ? PointerTrimShiftEnd : PointerTrimEnd;    break;
                }
            }
        }

    }
    getMouse().set(image);
}

boost::statechart::result Idle::start()
{
    getPlayer()->play();
    return discard_event();
}

boost::statechart::result Idle::leftDown()
{
    PointerPositionInfo info = getMouse().getInfo(getMouse().getLeftDownPosition());

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
        getSelection().updateOnLeftDown(info);
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

boost::statechart::result Idle::rightDown()
{
    PointerPositionInfo info = getMouse().getInfo(getMouse().getVirtualPosition());

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
        getSelection().updateOnRightClick(info);
    }
    return transit<StateRightDown>();
}

void Idle::addTransition(model::TransitionType type)
{
    PointerPositionInfo info = getMouse().getInfo(getMouse().getVirtualPosition());
    if (info.clip && 
        (info.clip->isA<model::VideoClip>() || info.clip->isA<model::AudioClip>()) )
    {
        pts left = getViewMap().getView(info.clip)->getLeftPixel();
        pts right = getViewMap().getView(info.clip)->getRightPixel();

        // Only if the mouse pointer is on a 'regular' clip, creating the transition is allowed.
        switch (info.logicalclipposition)
        {
        case ClipBegin:
        case ClipInterior:
        case ClipEnd:
            break;
        default:
            return;
        }

        // Check if there is already a transition at the given position
        switch (type)
        {
        case model::TransitionTypeIn:                               
        case model::TransitionTypeInOut: 
            if (info.clip->getInTransition() != nullptr) { return; }
            break;
        case model::TransitionTypeOut: 
        case model::TransitionTypeOutIn: 
            if (info.clip->getOutTransition() != nullptr) { return; }
            break;
        }

        ASSERT(info.track);
        model::TransitionPtr transition = info.track->isA<model::VideoTrack>() ? model::video::VideoTransitionFactory::get().getDefault() : model::audio::AudioTransitionFactory::get().getDefault();
        ::gui::timeline::command::createTransition(getSequence(), info.clip, type, transition);
    }
}

}}} // namespace