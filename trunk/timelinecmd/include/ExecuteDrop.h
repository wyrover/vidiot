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
        model::Clips clips;     ///< Must be contiguous

        friend std::ostream& operator<<( std::ostream& os, const Drop& obj );
    };
    typedef std::list<Drop> Drops;

    ExecuteDrop(gui::timeline::Timeline& timeline, model::Clips drags, Drops drops);

    ~ExecuteDrop();

    //////////////////////////////////////////////////////////////////////////
    // ACLIPEDIT INTERFACE
    //////////////////////////////////////////////////////////////////////////

    void initialize();

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    model::Clips mDrags;
    Drops mDrops;
};

}}} // namespace

#endif // EXECUTE_DROPS_H
