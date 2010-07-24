#include "StateIdle.h"

#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>
#include "GuiTimeLineClip.h"
#include "GuiTimeLine.h"
#include "StateTestDragStart.h"
#include "StateMovingCursor.h"
#include "StatePlaying.h"
#include "GuiTimeLineTrack.h"
#include "EmptyClip.h"
#include "Project.h"
#include "TimelineMoveClips.h"

namespace gui { namespace timeline { namespace state {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

Idle::Idle( my_context ctx ) // entry
:   my_base( ctx )
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
    GuiTimeLineClipPtr clip = outermost_context().timeline.findClip(evt.mPosition).get<0>();
    outermost_context().globals->selection.update(clip,evt.mWxEvent.ControlDown(),evt.mWxEvent.ShiftDown(),evt.mWxEvent.AltDown());
    if (clip && !clip->isEmpty())
    {
        outermost_context().globals->DragStartPosition = evt.mPosition;
        outermost_context().globals->DragStartClip = clip;
        return transit<TestDragStart>();
    }
    else
    {
        post_event(evt); // Handle this in the MovingCursor state.
        return transit<MovingCursor>();
    }
    return discard_event();
}

boost::statechart::result Idle::react( const EvMotion& evt )
{
    VAR_DEBUG(evt);
    MousePosition pos = outermost_context().globals->mousepointer.getLogicalPosition(evt.mPosition);
    MousePointerImage image = PointerNormal;
    switch (pos)
    {
    case MouseOnClipBegin:      image = evt.mWxEvent.ShiftDown() ? PointerTrimShiftBegin : PointerTrimBegin;    break;
    case MouseBetweenClips:     image = PointerMoveCut;     break;
    case MouseOnClipEnd:        image = evt.mWxEvent.ShiftDown() ? PointerTrimShiftEnd : PointerTrimEnd;    break;
    }
    outermost_context().globals->mousepointer.set(image);
    return discard_event();
}

boost::statechart::result Idle::react( const EvKeyDown& evt)
{
    switch (evt.mWxEvent.GetKeyCode())
    {
    case WXK_SPACE:
        return start();
    case WXK_DELETE:
        model::MoveParameters moves;
        deleteSelectedClips( moves, outermost_context().timeline.mVideoTracks);
        deleteSelectedClips( moves, outermost_context().timeline.mAudioTracks);
        model::Project::current()->Submit(new command::TimelineMoveClips(moves));
        break;
    }
    return discard_event();
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

boost::statechart::result Idle::start()
{
    outermost_context().timeline.mPlayer->play();
    return transit<Playing>();
}

void Idle::deleteSelectedClips(model::MoveParameters& moves, GuiTimeLineTracks tracks)
{
    BOOST_FOREACH( GuiTimeLineTrackPtr t, tracks)
    {
        model::MoveParameterPtr move;
        long nRemovedFrames = 0;
        BOOST_FOREACH( GuiTimeLineClipPtr c, t->getClips() )
        {
            model::ClipPtr modelClip = c->getClip();
            if (c->isSelected())
            {
                if (!move)
                {
                    move = boost::make_shared<model::MoveParameter>();
                    move->addTrack = t->getTrack();
                    move->removeTrack = t->getTrack();
                    nRemovedFrames = 0;
                }
                move->removeClips.push_back(c->getClip());
                nRemovedFrames += c->getClip()->getNumberOfFrames();
            }
            else
            {
                if (move) 
                { 
                    move->removePosition = c->getClip();
                    move->addPosition = c->getClip();
                    move->addClips.push_back(boost::make_shared<model::EmptyClip>(nRemovedFrames));
                    moves.push_back(move); 
                }
                // Reset for possible new region of clips
                move.reset();
            }
        }
        if (move) 
        { 
            move->removePosition.reset(); // Null ptr indicates 'at end'
            move->addPosition.reset(); // Null ptr indicates 'at end'
            move->addClips.push_back(boost::make_shared<model::EmptyClip>(nRemovedFrames));
            moves.push_back(move); 
        }
    }
}

}}} // namespace

