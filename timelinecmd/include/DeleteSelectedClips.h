#ifndef DELETE_SELECTED_CLIPS_H
#define DELETE_SELECTED_CLIPS_H

#include "AClipEdit.h"

namespace gui { namespace timeline { namespace command {

class DeleteSelectedClips
    :   public AClipEdit
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    DeleteSelectedClips(gui::timeline::Timeline& timeline);

    ~DeleteSelectedClips();

    //////////////////////////////////////////////////////////////////////////
    // ACLIPEDIT INTERFACE
    //////////////////////////////////////////////////////////////////////////

    void initialize();
};

}}} // namespace

#endif // DELETE_SELECTED_CLIPS_H
