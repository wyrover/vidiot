#ifndef SELECT_CLIPS_H
#define SELECT_CLIPS_H

#include "GuiPtr.h"
#include "ModelPtr.h"
#include "Part.h"
#include <set>

namespace gui { namespace timeline {

class Selection
    :   public Part
{
public:
    Selection();
    void update(model::ClipPtr clip, bool ctrlPressed, bool shiftPressed, bool altPressed);
    bool isSelected(model::ClipPtr clip) const;
    void setDrag(bool drag);
private:

    void selectClip(model::ClipPtr clip, bool selected);

    /**
    * Holds the clip which was previously (de)selected.
    */
    model::ClipPtr mPreviouslyClicked;

    std::set<model::ClipPtr> mSelected;
};

}} // namespace

#endif // SELECT_CLIPS_H
