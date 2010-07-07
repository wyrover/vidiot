#ifndef SELECT_CLIPS_H
#define SELECT_CLIPS_H

#include "GuiPtr.h"

class SelectClips
{
public:
    SelectClips(GuiTimeLine& timeline);
    void update(GuiTimeLineClipPtr clip, bool ctrlPressed, bool shiftPressed, bool altPressed);
private:
    GuiTimeLine& mTimeline;

    /**
    * Holds the clip which was previously (de)selected.
    */
    GuiTimeLineClipPtr mPreviouslyClicked;

};

#endif // SELECT_CLIPS_H
