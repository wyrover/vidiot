#ifndef SELECTION_H
#define SELECTION_H

#include <set>
#include <wx/event.h>
#include "GuiPtr.h"
#include "ModelPtr.h"
#include "Part.h"

namespace gui { namespace timeline {

class Selection
    :   public Part
    ,   public wxEvtHandler
{
public:
    Selection();

    //////////////////////////////////////////////////////////////////////////
    // MODEL EVENTS
    //////////////////////////////////////////////////////////////////////////

    void onClipsRemoved( model::EventRemoveClips& event );

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    void update(model::ClipPtr clip, bool ctrlPressed, bool shiftPressed, bool altPressed);
    bool isSelected(model::ClipPtr clip) const;
    //void setDrag(bool drag);

    /**
    * Deletes all selected clips.
    **/
    void deleteClips();

private:

    void selectClipAndLink(model::ClipPtr clip, bool selected);
    void selectClip(model::ClipPtr clip, bool selected);

    /**
    * Clip which was previously (de)selected.
    */
    model::ClipPtr mPreviouslyClicked;

    std::set<model::ClipPtr> mSelected;

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////
    
    void deleteFromTrack(model::MoveParameters& moves, model::Tracks tracks);
};

}} // namespace

#endif // SELECTION_H
