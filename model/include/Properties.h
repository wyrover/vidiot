#ifndef PROPERTIES_H
#define PROPERTIES_H

#include <wx/gdicmn.h>
#include <boost/shared_ptr.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/access.hpp>
#include "UtilFrameRate.h"

namespace model {

class Properties;
typedef boost::shared_ptr<Properties> PropertiesPtr;

class Properties
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    Properties();
    ~Properties();
    static PropertiesPtr get();

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    FrameRate getFrameRate() const;
    wxSize getVideoSize() const;

private:

    FrameRate mFrameRate;
    long mVideoWidth;
    long mVideoHeight;

    //////////////////////////////////////////////////////////////////////////
    // SERIALIZATION
    //////////////////////////////////////////////////////////////////////////

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);
};

} // namespace

// Workaround needed to prevent compile-time errors (mpl_assertion_in_line...) with gcc
//#include  <boost/preprocessor/slot/counter.hpp>
//#include BOOST____PP_UPDATE_COUNTER()
//#line BOOST_____PP_COUNTER
BOOST_CLASS_VERSION(model::Properties, 1)

#endif // PROPERTIES_H