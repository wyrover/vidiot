#include "SelectClips.h"

#include <boost/foreach.hpp>
#include "GuiTimeLine.h"
#include "GuiTimeLineClip.h"
#include "GuiTimeLineTrack.h"
#include "UtilLog.h"
#include "ViewMap.h"

namespace gui { namespace timeline {

SelectClips::SelectClips()
:   mSelected()
{
}

void SelectClips::update(GuiTimeLineClip* clip, bool ctrlPressed, bool shiftPressed, bool altPressed)
{
    model::TrackPtr track = clip ? clip->getTrack()->getTrack() : model::TrackPtr();

    // Must be determined before deselecting all clips.
    bool previousClickedClipWasSelected = mPreviouslyClicked ? getViewMap().getView(mPreviouslyClicked)->isSelected() : true;
    bool currentClickedClipIsSelected = clip ? clip->isSelected() : false;

    // Deselect all clips first, but only if control is not pressed.
    if (!ctrlPressed)
    {
        BOOST_FOREACH( model::ClipPtr c, mSelected )
        {
            getViewMap().getView(c)->setSelected(false);
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
                    selectClip(c ,!currentClickedClipIsSelected);
                }
                mPreviouslyClicked = clip->getClip();
            }
        }
        else if (shiftPressed)
        {
            // Range selection. Select from last selected clip until the current clip.
            // SelectClips value equals the state of the last selected clip. If that was
            // just selected, then the whole range is selected. If the last selected 
            // clip was deselected, then the whole range is deselected.

            model::ClipPtr otherend = (mPreviouslyClicked) ? mPreviouslyClicked : *(track->getClips().begin());

            model::ClipPtr firstclip;
            BOOST_FOREACH( model::ClipPtr c, track->getClips() )
            {
                /** /todo this does not work for multiple tracks yet. For multiple tracks the begin and endpoint should indicate both the x position (clip) as well as the y position (track) */
                if (!firstclip)
                {
                    if ((c == clip->getClip()) || (c == otherend))
                    {
                        firstclip = c;
                    }
                }
                if (firstclip)
                {
                    selectClip(c, previousClickedClipWasSelected);
                    if ((c != firstclip) && 
                        ((c == clip->getClip()) || (c == otherend)))
                    {
                        break; // Stop (de)selecting clips
                    }
                }
            }
        }
        else if (ctrlPressed)
        {
            // Control down implies 'toggle' select.
            selectClip(clip->getClip(), !currentClickedClipIsSelected);
            mPreviouslyClicked = clip->getClip();
        }
        else
        {
            selectClip(clip->getClip(), true);
            mPreviouslyClicked = clip->getClip();
        }
    }
    else
    {
        mPreviouslyClicked.reset();
    }
}

void SelectClips::setDrag(bool drag)
{
    BOOST_FOREACH(model::ClipPtr clip, mSelected)
    {
        getViewMap().getView(clip)->setBeingDragged(drag);
    }
}

void SelectClips::selectClip(model::ClipPtr clip, bool selected)
{
    model::ClipPtr link = clip->getLink();
    getViewMap().getView(clip)->setSelected(selected);
    getViewMap().getView(link)->setSelected(selected);

    if (selected)
    {
        mSelected.insert(clip);
        mSelected.insert(link);
    }
    else
    {
        mSelected.erase(clip);
        mSelected.erase(link);
    }

}

}} // namespace
