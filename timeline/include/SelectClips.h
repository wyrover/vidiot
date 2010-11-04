#ifndef SELECT_CLIPS_H
#define SELECT_CLIPS_H

#include "GuiPtr.h"
#include "ViewMap.h"
#include "ModelPtr.h"
#include <set>

namespace gui { namespace timeline {

class SelectClips
{
public:
    SelectClips(ViewMap& viewMap);
    void update(GuiTimeLineClipPtr clip, bool ctrlPressed, bool shiftPressed, bool altPressed);
    void setDrag(bool drag);
private:

    void selectClip(model::ClipPtr clip, bool selected);

    ViewMap& mViewMap;

    /**
    * Holds the clip which was previously (de)selected.
    */
    model::ClipPtr mPreviouslyClicked;

    std::set<model::ClipPtr> mSelected;
};

}} // namespace

#endif // SELECT_CLIPS_H
