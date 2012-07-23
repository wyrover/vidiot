#ifndef MODEL_I_CONTROL_H
#define MODEL_I_CONTROL_H

#include "UtilInt.h"

namespace model {

class IControl
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    virtual ~IControl() {};

    //////////////////////////////////////////////////////////////////////////
    // ICONTROL
    //////////////////////////////////////////////////////////////////////////

    virtual pts getLength() const = 0;
    virtual void moveTo(pts position) = 0;
    virtual wxString getDescription() const = 0; ///< Not called getName() to avoid conflict with Node::getName() for the Sequence class.
    virtual void clean() = 0;                    ///< Will be called to clean up resources. Used for minimizing required resources for the undo history.
};

} // namespace

#endif // MODEL_I_CONTROL_H