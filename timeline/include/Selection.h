#ifndef SELECTION_H
#define SELECTION_H

#include "GuiPtr.h"

class Selection
{
public:
    Selection(GuiTimeLine& timeline);
    void update(GuiTimeLineClipPtr clip, bool ctrlPressed, bool shiftPressed, bool altPressed);
private:
    GuiTimeLine& mTimeline;

    /**
    * Holds the clip which was previously (de)selected.
    */
    GuiTimeLineClipPtr mPreviouslyClicked;

};

#endif // SELECTION_H
