#ifndef MODEL_I_CONTROL_H
#define MODEL_I_CONTROL_H

#include <wx/string.h>
#include <boost/shared_ptr.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/version.hpp>
#include "UtilCloneable.h"
#include "UtilInt.h"

namespace model {

class IControl
    :   public ICloneable
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    virtual ~IControl() {};

    //////////////////////////////////////////////////////////////////////////
    // ICLONEABLE
    //////////////////////////////////////////////////////////////////////////

    virtual IControl* clone() = 0;

    //////////////////////////////////////////////////////////////////////////
    // ICONTROL
    //////////////////////////////////////////////////////////////////////////

    virtual pts getLength() = 0;
    virtual void moveTo(pts position) = 0;
    virtual wxString getDescription() const = 0; ///< Not called getName() to avoid conflict with AProjectViewNode::getName() for the Sequence class.
    virtual void clean() = 0;                    ///< Will be called to clean up resources. Used for minimizing required resources for the undo history.

    //////////////////////////////////////////////////////////////////////////
    // SERIALIZATION
    //////////////////////////////////////////////////////////////////////////

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
    }
};

typedef boost::shared_ptr<IControl> IControlPtr;

} // namespace

// Workaround needed to prevent compile-time errors (mpl_assertion_in_line...) with gcc
//#include  <boost/preprocessor/slot/counter.hpp>
//#include BOOST____PP_UPDATE_COUNTER()
//#line BOOST_____PP_COUNTER
BOOST_CLASS_VERSION(model::IControl, 1)

#endif // MODEL_I_CONTROL_H
