#ifndef MODEL_I_CONTROL_H
#define MODEL_I_CONTROL_H

#include <wx/string.h>
#include <boost/serialization/access.hpp>
#include <boost/serialization/assume_abstract.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/tracking.hpp>
#include <boost/serialization/version.hpp>
#include "ModelPtr.h"
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

    virtual pts getNumberOfFrames() = 0;
    virtual void moveTo(pts position) = 0;
    virtual wxString getDescription() const = 0; ///< Not called getName() to avoid conflict with AProjectViewNode::getName() for the Sequence class.

    //////////////////////////////////////////////////////////////////////////
    // SERIALIZATION
    //////////////////////////////////////////////////////////////////////////

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
    }
};

} // namespace

BOOST_SERIALIZATION_ASSUME_ABSTRACT(model::IControl)
// Workaround needed to prevent compile-time errors (mpl_assertion_in_line...) with gcc
//#include  <boost/preprocessor/slot/counter.hpp>
//#include BOOST____PP_UPDATE_COUNTER()
//#line BOOST_____PP_COUNTER
BOOST_CLASS_VERSION(model::IControl, 1)
BOOST_CLASS_EXPORT(model::IControl)
BOOST_CLASS_TRACKING(model::IControl, boost::serialization::track_always)

#endif // MODEL_I_CONTROL_H
