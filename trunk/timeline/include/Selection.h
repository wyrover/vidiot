#ifndef SELECTION_H
#define SELECTION_H

#include "Part.h"
#include "PositionInfo.h"

namespace model {
    class EventRemoveClips;
}

namespace gui { namespace timeline {

class Selection
    :   public wxEvtHandler // MUST BE FIRST INHERITED CLASS FOR WXWIDGETS EVENTS TO BE RECEIVED.
    ,   public Part
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    Selection(Timeline* timeline);
    virtual ~Selection();

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    void updateOnLeftClick(const PointerPositionInfo& info);
    void updateOnRightClick(model::IClipPtr clip);

    /// Deletes all selected clips.
    void deleteClips();

    /// Unselect all clips
    void unselectAll();

    /// Change the selection into the given list of clips.
    /// List may contain 0-ptrs, these are ignored.
    /// Any other selection is cleared.
    void change(model::IClips selection);

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    model::IClipPtr mPreviouslyClicked;      ///< Clip which was previously (de)selected.

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    void selectClipAndLink(model::IClipPtr clip, bool selected);
    void selectClip(model::IClipPtr clip, bool selected);
    void setPreviouslyClicked(model::IClipPtr clip);
};

}} // namespace

#endif // SELECTION_H