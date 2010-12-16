#include "Selection.h"

#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>
#include "Timeline.h"
#include "ClipView.h"
#include "Track.h"
#include "TrackView.h"
#include "UtilLog.h"
#include "ViewMap.h"
#include "Sequence.h"
#include "EmptyClip.h"
#include "TimelineMoveClips.h"

namespace gui { namespace timeline {

Selection::Selection(Timeline* timeline)
:   wxEvtHandler()
,   Part(timeline)
,   mSelected()
,   mPreviouslyClicked()
{
}

//////////////////////////////////////////////////////////////////////////
// MODEL EVENTS
//////////////////////////////////////////////////////////////////////////

void Selection::onClipsRemoved( model::EventRemoveClips& event )
{
    model::Clips clips = event.getValue().removeClips;
    if (find(clips.begin(),clips.end(),mPreviouslyClicked) != clips.end())
    {
        mPreviouslyClicked.reset();
    }

    BOOST_FOREACH( model::ClipPtr clip, event.getValue().removeClips )
    {
        mSelected.erase(clip);
    }
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

void Selection::update(model::ClipPtr clip, bool ctrlPressed, bool shiftPressed, bool altPressed)
{
    model::TrackPtr track = clip ? clip->getTrack() : model::TrackPtr();

    // Must be determined before deselecting all clips.
    bool previousClickedClipWasSelected = mPreviouslyClicked ? mPreviouslyClicked->getSelected() : true;
    bool currentClickedClipIsSelected = clip ? clip->getSelected() : false;

    // Deselect all clips first, but only if control is not pressed.
    if (!ctrlPressed)
    {
        BOOST_FOREACH( model::ClipPtr c, mSelected )
        {
            c->setSelected(false);
        }
        mSelected.clear();
    }

    if (clip)
    {
        if (altPressed)
        {
            model::ClipPtr firstclip;
            model::ClipPtr lastclip;

            /** /todo this does not work for multiple tracks yet. For multiple tracks the begin and endpoint should indicate both the x position (clip) as well as the y position (track) */
            BOOST_FOREACH( model::ClipPtr c, track->getClips() )
            {
                if (c == mPreviouslyClicked)
                {
                    firstclip = c;
                }
                if (firstclip)
                {
                    selectClipAndLink(c ,!currentClickedClipIsSelected);
                }
                mPreviouslyClicked = clip;
            }
        }
        else if (shiftPressed)
        {
            // Range selection. Select from last selected clip until the current clip.
            // Selection value equals the state of the last selected clip. If that was
            // just selected, then the whole range is selected. If the last selected 
            // clip was deselected, then the whole range is deselected.

            model::ClipPtr otherend = (mPreviouslyClicked) ? mPreviouslyClicked : *(track->getClips().begin());

            model::ClipPtr firstclip;
            BOOST_FOREACH( model::ClipPtr c, track->getClips() )
            {
                /** /todo this does not work for multiple tracks yet. For multiple tracks the begin and endpoint should indicate both the x position (clip) as well as the y position (track) */
                if (!firstclip)
                {
                    if ((c == clip) || (c == otherend))
                    {
                        firstclip = c;
                    }
                }
                if (firstclip)
                {
                    selectClipAndLink(c, previousClickedClipWasSelected);
                    if ((c != firstclip) && 
                        ((c == clip) || (c == otherend)))
                    {
                        break; // Stop (de)selecting clips
                    }
                }
            }
        }
        else if (ctrlPressed)
        {
            // Control down implies 'toggle' select.
            selectClipAndLink(clip, !currentClickedClipIsSelected);
            mPreviouslyClicked = clip;
        }
        else
        {
            selectClipAndLink(clip, true);
            mPreviouslyClicked = clip;
        }
    }
    else
    {
        mPreviouslyClicked.reset();
    }
}

void Selection::deleteClips()
{
    model::MoveParameters moves;
    deleteFromTrack( moves, getSequence()->getVideoTracks());
    deleteFromTrack( moves, getSequence()->getAudioTracks());

    mPreviouslyClicked.reset();
    mSelected.clear(); // Since these clips are going to be removed, they may not be referenced anymore.
    getTimeline().Submit(new command::TimelineMoveClips(getTimeline(),moves));
}

const std::set<model::ClipPtr>& Selection::getClips() const
{
    return mSelected;
}

void Selection::selectClipAndLink(model::ClipPtr clip, bool selected)
{
    selectClip(clip,selected);
    model::ClipPtr link = clip->getLink();
    if (link)
    {
        selectClip(link,selected);
    }
}

void Selection::selectClip(model::ClipPtr clip, bool selected)
{
    clip->setSelected(selected);
    if (selected)
    {
        mSelected.insert(clip);
        clip->getTrack()->Bind(model::EVENT_REMOVE_CLIPS, &Selection::onClipsRemoved, this); /** todo just register for all tracks... */
    }
    else
    {
        mSelected.erase(clip);
    }
    getViewMap().getView(clip)->invalidateBitmap();
}

void Selection::deleteFromTrack(model::MoveParameters& moves, model::Tracks tracks)
{
    BOOST_FOREACH( model::TrackPtr track, tracks)
    {
        model::MoveParameterPtr move;
        long nRemovedFrames = 0;
        BOOST_FOREACH( model::ClipPtr clip, track->getClips() )
        {
            ClipView* c = getViewMap().getView(clip);
            if (clip->getSelected())
            {
                if (!move)
                {
                    move = boost::make_shared<model::MoveParameter>();
                    move->addTrack = track;
                    move->removeTrack = track;
                    nRemovedFrames = 0;
                }
                move->removeClips.push_back(clip);
                nRemovedFrames += clip->getNumberOfFrames();
            }
            else
            {
                if (move) 
                {
                    move->removePosition = clip;
                    move->addPosition = clip;
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

}} // namespace
