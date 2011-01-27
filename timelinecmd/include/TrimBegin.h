#ifndef TRIM_BEGIN_H
#define TRIM_BEGIN_H

#include "AClipEdit.h"

namespace gui { namespace timeline { namespace command {

class TrimBegin 
    :   public AClipEdit
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    TrimBegin(gui::timeline::Timeline& timeline, model::ClipPtr clip, pts diff);

    ~TrimBegin();

    //////////////////////////////////////////////////////////////////////////
    // ACLIPEDIT INTERFACE
    //////////////////////////////////////////////////////////////////////////

    void initialize();

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    model::ClipPtr mClip;
    pts mDiff;

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    void splittrack(model::Tracks tracks, pts position, ReplacementMap& linkmapper);
};

}}} // namespace

#endif // TRIM_BEGIN_H