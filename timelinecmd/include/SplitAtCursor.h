#ifndef SPLIT_AT_CURSOR_H
#define SPLIT_AT_CURSOR_H

#include "AClipEdit.h"

namespace gui { namespace timeline { namespace command {

class SplitAtCursor 
    :   public AClipEdit
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    SplitAtCursor(gui::timeline::Timeline& timeline);

    ~SplitAtCursor();

    //////////////////////////////////////////////////////////////////////////
    // ACLIPEDIT INTERFACE
    //////////////////////////////////////////////////////////////////////////

    void initialize();

private:

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    void splittrack(model::Tracks tracks, pts position, ReplacementMap& linkmapper);
};

}}} // namespace

#endif // SPLIT_AT_CURSOR_H