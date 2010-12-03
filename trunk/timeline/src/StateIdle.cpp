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
#include "TimelineMoveClips.h"
#include "MousePointer.h"
#include "Selection.h"
#include "StateMoveDivider.h"
#include "UtilLog.h"
#include "PositionInfo.h"

namespace gui { namespace timeline { namespace state {

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
    else
    {
        getSelection().update(info.clip,evt.mWxEvent.ControlDown(),evt.mWxEvent.ShiftDown(),evt.mWxEvent.AltDown());
        if (info.clip && !info.clip->isA<model::EmptyClip>())
        {
            outermost_context().globals->DragStartPosition = evt.mPosition;
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
     switch (evt.mWxEvent.GetKeyCode())
    {
    case WXK_SPACE:
        return start();
        break;
    case WXK_DELETE:
        {
            getSelection().deleteClips();
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

                long pts = getZoom().pixelsToPts(evt.mPosition.x);
                model::Tracks tracks = getSequence()->getTracks();
                BOOST_FOREACH( model::TrackPtr track,  getSequence()->getTracks() )
//                BOOST_FOREACH( TrackView* t, outermost_context().timeline.getTracks())
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

                model::Project::current()->Submit(new command::TimelineMoveClips(getTimeline(),moves));
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
    getPlayer()->play();
    return transit<Playing>();
}

}}} // namespace

