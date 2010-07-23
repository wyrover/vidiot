#include "SelectClips.h"

#include <boost/foreach.hpp>
#include "GuiTimeLine.h"
#include "GuiTimeLineClip.h"
#include "UtilLog.h"

namespace gui { namespace timeline {

SelectClips::SelectClips(GuiTimeLine& timeline)
:   mTimeline(timeline)
{

}
void SelectClips::update(GuiTimeLineClipPtr clip, bool ctrlPressed, bool shiftPressed, bool altPressed)
{
    // Must be determined before deselecting all clips.
    bool previousClickedClipWasSelected = mPreviouslyClicked ? mPreviouslyClicked->isSelected() : true;
    bool currentClickedClipIsSelected = clip ? clip->isSelected() : false;

    // Deselect all clips first, but only if control is not pressed.
    if (!ctrlPressed)
    {
        BOOST_FOREACH( GuiTimeLineClipPtr c, mTimeline.getClips() )
        {
            c->setSelected(false);
        }
    }

    if (clip)
    {
        if (altPressed)
        {
            // SelectClips till end. 

            GuiTimeLineClipPtr firstclip;
            GuiTimeLineClipPtr lastclip;
            BOOST_FOREACH( GuiTimeLineClipPtr c, mTimeline.getClips() )
            {
                /** /todo this does not work for multiple tracks yet. For multiple tracks the begin and endpoint should indicate both the x position (clip) as well as the y position (track) */
                if (c == mPreviouslyClicked)
                {
                    firstclip = c;
                }
                if (firstclip)
                {
                    c->setSelected(!currentClickedClipIsSelected);
                }
                mPreviouslyClicked = clip;
            }
        }
        else if (shiftPressed)
        {
            // Range selection. Select from last selected clip until the current clip.
            // SelectClips value equals the state of the last selected clip. If that was
            // just selected, then the whole range is selected. If the last selected 
            // clip was deselected, then the whole range is deselected.

            GuiTimeLineClipPtr otherend = (mPreviouslyClicked) ? mPreviouslyClicked : *(mTimeline.getClips().begin());

            GuiTimeLineClipPtr firstclip;
            BOOST_FOREACH( GuiTimeLineClipPtr c, mTimeline.getClips() )
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
                    c->setSelected(previousClickedClipWasSelected);
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
            clip->setSelected(!currentClickedClipIsSelected);
            mPreviouslyClicked = clip;
        }
        else
        {
            clip->setSelected(true);
            mPreviouslyClicked = clip;
        }
    }
    else
    {
        mPreviouslyClicked.reset();
    }
}

}} // namespace
