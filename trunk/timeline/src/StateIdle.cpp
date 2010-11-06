#include "StateIdle.h"

#include <boost/assign/list_of.hpp>
#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>
#include "GuiTimeLineClip.h"
#include "GuiTimeLine.h"
#include "StateTestDragStart.h"
#include "StateMovingCursor.h"
#include "StatePlaying.h"
#include "GuiPlayer.h"
#include "GuiTimeLineZoom.h"
#include "GuiTimeLineTrack.h"
#include "EmptyClip.h"
#include "Track.h"
#include "Sequence.h"
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
    outermost_context().timeline.SetFocus(); /** @todo make more generic, for all states */
    GuiTimeLineClipPtr clip = outermost_context().timeline.findClip(evt.mPosition);
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
    PointerPositionInfo info =  outermost_context().timeline.getPointerInfo(evt.mPosition);
    MousePointerImage image = PointerNormal;
    if (info.clip)
    {
        switch (info.logicalclipposition)
        {
        case ClipBegin:      image = evt.mWxEvent.ShiftDown() ? PointerTrimShiftBegin : PointerTrimBegin;    break;
        case ClipBetween:    image = PointerMoveCut;     break;
        case ClipEnd:        image = evt.mWxEvent.ShiftDown() ? PointerTrimShiftEnd : PointerTrimEnd;    break;
        }
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
        break;
    case WXK_DELETE:
        {
            model::MoveParameters moves;
            deleteSelectedClips( moves, outermost_context().timeline.mVideoTracks);
            deleteSelectedClips( moves, outermost_context().timeline.mAudioTracks);
            model::Project::current()->Submit(new command::TimelineMoveClips(moves));
            break;
        }
    case 'K':
        {
            if (evt.mWxEvent.ControlDown())
            {
                LOG_INFO << "Split clip";
                model::MoveParameters moves;

                // todo method: splitclipat(int pos)
                //LinkReplacer linkReplacer;
                typedef std::map<model::ClipPtr, model::Clips > ReplacementMap;
                ReplacementMap mConversion;

                long pts = outermost_context().timeline.mZoom.pixelsToPts(evt.mPosition.x);
                model::Tracks tracks = outermost_context().timeline.getSequence()->getTracks();
                BOOST_FOREACH( model::TrackPtr track,  outermost_context().timeline.getSequence()->getTracks() )
//                BOOST_FOREACH( GuiTimeLineTrackPtr t, outermost_context().timeline.getTracks())
                {
                    //model::TrackPtr track = t->getTrack();
                    model::ClipPtr splitclip = track->getClip(pts);

                    if (splitclip && !splitclip->isA<model::EmptyClip>())
                    {
                        // Clip found in track
                        boost::int64_t ptsoffset = track->getStartFrameNumber(splitclip);
                        boost::int64_t splitoffset = pts - ptsoffset;

                        if (splitoffset != 0 && splitoffset != splitclip->getNumberOfFrames())
                        {
                            // If the new cut is exactly at the beginning, or exactly at the end of the
                            // found clip, then there is no need to add a cut, since there is already
                            // one at the 'pts' specified position.

                            model::ClipPtr first = make_cloned<model::Clip>(splitclip);
                            first->setLength(splitoffset);
                            model::ClipPtr second = make_cloned<model::Clip>(splitclip);
                            second->adjustBeginPoint(splitoffset);

                            //linkReplacer.addReplacement(splitclip, boost::assign::list_of(first)(second) );
                            ASSERT(mConversion.find(splitclip) == mConversion.end());
                            mConversion[ splitclip ] = boost::assign::list_of(first)(second);

                            moves.push_back(boost::make_shared<model::MoveParameter>(
                                track, 
                                track->getNextClip(splitclip), 
                                boost::assign::list_of(first)(second),
                                track, 
                                track->getNextClip(splitclip), 
                                boost::assign::list_of(splitclip) ));
                        }
                    }
                }

                // For all replaced clips, ensure that the linked clip is also replaced,
                // at least with just a plain clone of the original link. This is needed to
                // avoid having these links 'dangling' after removal.
                BOOST_FOREACH( ReplacementMap::value_type link, mConversion )
                {
                    model::ClipPtr original = link.first;
                    model::ClipPtr originallink = original->getLink();
                    if (mConversion.find(originallink) == mConversion.end())
                    {
                        model::ClipPtr clone = make_cloned<model::Clip>(originallink);
                        clone->setLink(model::ClipPtr());
                        mConversion[ originallink ] = boost::assign::list_of(clone);

                        model::TrackPtr linktrack = originallink->getTrack();
                        moves.push_back(boost::make_shared<model::MoveParameter>(
                            linktrack, 
                            linktrack->getNextClip(originallink), 
                            boost::assign::list_of(clone),
                            linktrack, 
                            linktrack->getNextClip(originallink), 
                            boost::assign::list_of(originallink) ));
                    }
                }


                // At this point all clips AND their original links are part of the
                // replacement map. Now the replacements for a clip are linked to the
                // replacements of its link.
                BOOST_FOREACH( ReplacementMap::value_type link, mConversion )
                {
                    model::ClipPtr clip1 = link.first;
                    model::Clips new1 = link.second;
                    model::Clips::iterator it1 = new1.begin();

                    model::ClipPtr clip2 = clip1->getLink();
                    model::Clips new2 = mConversion[clip2];
                    model::Clips::iterator it2 = new2.begin();

                    while ( it1 != new1.end() && it2 != new2.end() )
                    {
                        (*it1)->setLink(*it2);
                        (*it2)->setLink(*it1);
                        ++it1;
                        ++it2;
                    }
                    // For all remaining clips in both lists: not linked.
                    while ( it1 != new1.end() )
                    {
                        (*it1)->setLink(model::ClipPtr());
                        ++it1;
                    }
                    while ( it2 != new2.end() )
                    {
                        (*it2)->setLink(model::ClipPtr());
                        ++it2;
                    }
                }

                model::Project::current()->Submit(new command::TimelineMoveClips(moves));
            }
        }
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
        BOOST_FOREACH( model::ClipPtr clip, t->getTrack()->getClips() )
        {
            GuiTimeLineClipPtr c = outermost_context().globals->mViewMap.ModelToView(clip);
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

