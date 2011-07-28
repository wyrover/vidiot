#ifndef CREATE_TRANSITION_H
#define CREATE_TRANSITION_H

#include <wx/gdicmn.h>
#include "AClipEdit.h"


namespace gui { namespace timeline { namespace command {

class CreateTransition 
    :   public AClipEdit
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    CreateTransition(model::SequencePtr sequence, wxPoint position);

    ~CreateTransition();

    //////////////////////////////////////////////////////////////////////////
    // ACLIPEDIT INTERFACE
    //////////////////////////////////////////////////////////////////////////

    void initialize();

    //////////////////////////////////////////////////////////////////////////
    // 
    //////////////////////////////////////////////////////////////////////////

    /// Determine if a transition is possible
    bool isPossible();

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    model::IClipPtr mLeft;
    model::IClipPtr mRight;

    pts mLeftSize;
    pts mRightSize;

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<<( std::ostream& os, const CreateTransition& obj );
};

}}} // namespace

#endif // CREATE_TRANSITION_H
