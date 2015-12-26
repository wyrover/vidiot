// Copyright 2013-2015 Eric Raijmakers.
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
#include "ClipInterval.h"
#include "ClipView.h"
#include "CreateTransitionHelper.h"
#include "Cursor.h"
#include "Details.h"
#include "DetailsClip.h"
#include "Drag.h"
#include "EmptyClip.h"
#include "EventClipboard.h"
#include "EventDrag.h"
#include "EventKey.h"
#include "EventMouse.h"
#include "EventPart.h"
#include "Intervals.h"
#include "Keyboard.h"
#include "Menu.h"
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
#include "StateMoveKeyFrame.h"
#include "StateMoveTrackDivider.h"
#include "StateMovingCursor.h"
#include "StatePlaying.h"
#include "StateRightDown.h"
#include "StateScrolling.h"
#include "StateTrim.h"
#include "Timeline.h"
#include "TimelineClipboard.h"
#include "Transition.h"
#include "Track.h"
#include "UtilLog.h"
#include "UtilLogStl.h"
#include "VideoTrack.h"
#include "VideoTransitionFactory.h"
#include "ViewMap.h"
#include "Zoom.h"

namespace gui { namespace timeline { namespace state {

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

boost::statechart::result Idle::react(const EvCut& evt)
{
    VAR_DEBUG(evt);
    getClipboard().onCut();
    return discard_event();
}

boost::statechart::result Idle::react(const EvCopy& evt)
{
    VAR_DEBUG(evt);
    getClipboard().onCopy();
    return discard_event();
}

boost::statechart::result Idle::react(const EvPaste& evt)
{
    VAR_DEBUG(evt);
    getClipboard().onPaste();
    return discard_event();
}

boost::statechart::result Idle::react(const EvLeftDown& evt)
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
        {
            evt.consumed();
            return start();
        }
        case WXK_DELETE:
        {
            evt.consumed();
            getSelection().deleteClips(getKeyboard().getShiftDown());
            break;
        }
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case '0':
        {
            getDetails().get<DetailsClip>()->onTimelineKey(evt.KeyCode);
            evt.consumed();
            break;
        }
        case 'b':
        case 'B':
        {
            evt.consumed();
            cmd::SplitAtCursorAndTrim(getSequence(), true);
            break;
        }
        case 'c':
        case 'C':
        {
            PointerPositionInfo info = getMouse().getInfo(getMouse().getVirtualPosition());
            if (info.clip && info.clip->isA<model::EmptyClip>())
            {
                getIntervals().deleteEmptyClip(info.clip);
            }
            break;
        }
        case 'e':
        case 'E':
        {
            evt.consumed();
            cmd::SplitAtCursorAndTrim(getSequence(), false);
            break;
        }
        case 'i':
        case 'I':
        {
            evt.consumed();
            addTransition(model::TransitionTypeFadeIn);
            break;
        }
        case 'n':
        case 'N':
        {
            evt.consumed();
            addTransition(model::TransitionTypeFadeOutToNext);
            break;
        }
        case 'o':
        case 'O':
        {
            evt.consumed();
            addTransition(model::TransitionTypeFadeOut);
            break;
        }
        case 'p':
        case 'P':
        {
            evt.consumed();
            addTransition(model::TransitionTypeFadeInFromPrevious);
            break;
        }
        case 's':
        case 'S':
        {
            if (!evt.CtrlDown)
            {
                evt.consumed();
                model::ProjectModification::submitIfPossible(new cmd::SplitAtCursor(getSequence()));
            }
            break;
        }
        case '-':
        {
            evt.consumed();
            getZoom().change(evt.CtrlDown ? -1000 : -1);
            break;
        }
        case '=':
        {
            evt.consumed();
            getZoom().change(evt.CtrlDown ? 1000 : 1);
            break;
        }
        case 'v':
        case 'V':
        {
            evt.consumed();
            getCursor().focus();
            break;
        }
        case WXK_LEFT:
        {
            evt.consumed();
            evt.CtrlDown ? getCursor().prevCut() : getCursor().prevFrame();
            break;
        }
        case WXK_RIGHT:
        {
            evt.consumed();
            evt.CtrlDown ? getCursor().nextCut() : getCursor().nextFrame();
            break;
        }
        case WXK_HOME:
        {
            evt.consumed();
            getCursor().home();
            break;
        }
        case WXK_END:
        {
            evt.consumed();
            getCursor().end();
            break;
        }
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
            if (info.keyframe)
            {
                // Do not change cursor. Otherwise, it's hard to click on the key frame.
            }
            else if (info.clip)
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
    PointerPositionInfo info{ getMouse().getInfo(getMouse().getLeftDownPosition()) };

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
        if (info.keyframe)
        {
            // Move the cursor to the key frame to ensure that the details view shows this specific key frame.
            model::ClipIntervalPtr interval{ boost::dynamic_pointer_cast<model::ClipInterval>(info.getLogicalClip()) };
            ASSERT_NONZERO(interval);
            std::map<pts, model::KeyFramePtr> keyFrames{ interval->getKeyFramesOfPerceivedClip() };
            size_t count{ *info.keyframe };
            std::map<pts, model::KeyFramePtr>::const_iterator it{ std::next(keyFrames.begin(), count) };
            ASSERT(it != keyFrames.end())(keyFrames)(count)(info);
            getCursor().setLogicalPosition(interval->getPerceivedLeftPts() + it->first);
            return transit<MoveKeyFrame>();
        }
        else if (info.clip && !info.clip->isA<model::EmptyClip>())
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
        case model::TransitionTypeFadeIn:
        case model::TransitionTypeFadeInFromPrevious:
            if (info.clip->getInTransition() != nullptr) { return; }
            break;
        case model::TransitionTypeFadeOut:
        case model::TransitionTypeFadeOutToNext:
            if (info.clip->getOutTransition() != nullptr) { return; }
            break;
        }

        ASSERT(info.track);
        model::TransitionPtr transition = info.track->isA<model::VideoTrack>() ? model::video::VideoTransitionFactory::get().getDefault() : model::audio::AudioTransitionFactory::get().getDefault();
        ::gui::timeline::cmd::createTransition(getSequence(), info.clip, type, transition);
    }
}

}}} // namespace
