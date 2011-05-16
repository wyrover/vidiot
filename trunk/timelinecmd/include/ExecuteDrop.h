#ifndef EXECUTE_DROPS_H
#define EXECUTE_DROPS_H

#include "AClipEdit.h"

namespace gui { namespace timeline { namespace command {

class ExecuteDrop 
    :   public AClipEdit
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    struct Drop
    {
        model::TrackPtr track;
        pts position;
        model::IClips clips;     ///< Must be contiguous

        friend std::ostream& operator<<( std::ostream& os, const Drop& obj );
    };
    typedef std::list<Drop> Drops;

    ExecuteDrop(gui::timeline::Timeline& timeline, model::IClips drags, Drops drops, pts shiftPosition = -1, pts shiftSize = -1);

    ~ExecuteDrop();

    //////////////////////////////////////////////////////////////////////////
    // ACLIPEDIT INTERFACE
    //////////////////////////////////////////////////////////////////////////

    void initialize();

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    model::IClips mDrags;
    Drops mDrops;
    pts mShiftPosition;
    pts mShiftSize;
};

}}} // namespace

#endif // EXECUTE_DROPS_H
