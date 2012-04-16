#include "Selection.h"

#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>
#include "Timeline.h"
#include "DetailsView.h" // todo move the 'focused item handling' to one dedicated class? Or make detailsview handle that (not the timeline)
#include "ClipView.h"
#include "Track.h"
#include "TrackView.h"
#include "Transition.h"
#include "UtilLog.h"
#include "ViewMap.h"
#include "Sequence.h"
#include "EmptyClip.h"
#include "DeleteSelectedClips.h"

namespace gui { namespace timeline {

Selection::Selection(Timeline* timeline)
:   wxEvtHandler()
,   Part(timeline)
,   mPreviouslyClicked()
{
    VAR_DEBUG(this);
}

Selection::~Selection()
{
    VAR_DEBUG(this);
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

void Selection::updateOnLeftClick(const PointerPositionInfo& info)
{
    bool ctrlPressed = wxGetMouseState().ControlDown();
    bool shiftPressed = wxGetMouseState().ShiftDown();
    bool altPressed = wxGetMouseState().AltDown();

    VAR_DEBUG(info)(ctrlPressed)(shiftPressed)(altPressed);

    // Must be determined before deselecting all clips.
    bool previousClickedClipWasSelected = true;
    if (mPreviouslyClicked)
    {
        if (!mPreviouslyClicked->getTrack())
        {
            // Apparently, this clip was removed from the track (undo/redo/delete).
            // It may no longer be used.
            mPreviouslyClicked = model::IClipPtr(); // reset
        }
        else
        {
            previousClickedClipWasSelected = mPreviouslyClicked->getSelected();
        }
    }

    // todo temp for test
    if (info.clip && info.track)
    {
        DetailsView::get().focus(info.clip);
    }

    // Determine the 'logically clicked' clip and track
    model::IClipPtr clip = info.clip;
    model::TrackPtr track = info.track;
    if (clip && clip->isA<model::Transition>())
    {
        switch (info.logicalclipposition)
        {
        case TransitionLeftClipInterior:
        case TransitionLeftClipEnd:
            clip = info.clip->getPrev();
            ASSERT(clip);
            break;
        case TransitionRightClipBegin:
        case TransitionRightClipInterior:
            clip = info.clip->getNext();
            ASSERT(clip);
            break;
        case TransitionBegin:
        case TransitionInterior:
        case TransitionEnd:
            // Transition is actually clicked
            break;
        default:
            FATAL("Unexpected logical clip position.");
        }
    }

    bool currentClickedClipIsSelected = clip ? clip->getSelected() : false;

    // Deselect all clips first, but only if control is not pressed.
    if (!ctrlPressed)
    {
        BOOST_FOREACH( model::IClipPtr c, getSequence()->getSelectedClips() )
        {
            c->setSelected(false);
        }
    }

    if (clip)
    {
        ASSERT(track);
        if (altPressed)
        {
            // Select until the end
            model::IClipPtr firstclip;
            BOOST_FOREACH( model::IClipPtr c, track->getClips() )
            {
                if (c == clip)
                {
                    firstclip = c;
                }
                if (firstclip)
                {
                    selectClipAndLink(c , !currentClickedClipIsSelected);
                }
            }
            setPreviouslyClicked(clip);
        }
        else if (shiftPressed)
        {
            // Range selection. Select from last selected clip until the current clip.
            // Selection value equals the state of the last selected clip. If that was
            // just selected, then the whole range is selected. If the last selected
            // clip was deselected, then the whole range is deselected.

            // If no previous clip was selected: select from the start of the track
            model::IClipPtr otherend = (mPreviouslyClicked) ? mPreviouslyClicked : *(track->getClips().begin());

            model::IClipPtr firstclip;
            BOOST_FOREACH( model::IClipPtr c, track->getClips() )
            {
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
            setPreviouslyClicked(clip);
        }
        else
        {
            selectClipAndLink(clip, true);
            setPreviouslyClicked(clip);
        }
    }
    else
    {
        setPreviouslyClicked(model::IClipPtr()); // reset
    }
}

void Selection::updateOnRightClick(model::IClipPtr clip)
{
    bool ctrlPressed = wxGetMouseState().ControlDown();
    bool shiftPressed = wxGetMouseState().ShiftDown();
    bool altPressed = wxGetMouseState().AltDown();

    VAR_DEBUG(clip)(ctrlPressed)(shiftPressed)(altPressed);

    model::TrackPtr track = clip ? clip->getTrack() : model::TrackPtr();

    // Must be determined before deselecting all clips.
    bool currentClickedClipIsSelected = clip ? clip->getSelected() : false;

    // Deselect clips first, in certain cases
    if (!ctrlPressed && (!clip || !clip->getSelected()))
    {
        BOOST_FOREACH( model::IClipPtr c, getSequence()->getSelectedClips() )
        {
            c->setSelected(false);
        }
    }

    if (clip)
    {
        selectClipAndLink(clip, true);
        setPreviouslyClicked(clip);
    }
    else
    {
        setPreviouslyClicked(model::IClipPtr()); // reset
    }
}

void Selection::deleteClips()
{
    setPreviouslyClicked(model::IClipPtr()); // reset
    getTimeline().Submit(new command::DeleteSelectedClips(getSequence()));
}

void Selection::unselectAll()
{
    BOOST_FOREACH( model::TrackPtr track, getSequence()->getTracks() )
    {
        BOOST_FOREACH( model::IClipPtr clip, track->getClips() )
        {
            selectClip(clip,false);
        }
    }
    setPreviouslyClicked(model::IClipPtr()); // reset
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void Selection::selectClipAndLink(model::IClipPtr clip, bool selected)
{
    selectClip(clip,selected);
    model::IClipPtr link = clip->getLink();
    if (link)
    {
        selectClip(link,selected);
    }
}

void Selection::selectClip(model::IClipPtr clip, bool selected)
{
    clip->setSelected(selected);
}

void Selection::setPreviouslyClicked(model::IClipPtr clip)
{
    mPreviouslyClicked = clip;
}

}} // namespace