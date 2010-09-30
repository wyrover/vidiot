#ifndef MODEL_I_VIDEO_H
#define MODEL_I_VIDEO_H

#include "VideoFrame.h"
#include <boost/serialization/access.hpp>
#include <boost/serialization/assume_abstract.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/tracking.hpp>
#include <boost/serialization/version.hpp>
#include "FrameRate.h"
#include "ModelPtr.h"

namespace model {

class IVideo
{
public:

    //////////////////////////////////////////////////////////////////////////
    // IVIDEO
    //////////////////////////////////////////////////////////////////////////

    virtual VideoFramePtr getNextVideo(int requestedWidth, int requestedHeight, bool alpha = true) = 0;

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

BOOST_SERIALIZATION_ASSUME_ABSTRACT(model::IVideo)
// Workaround needed to prevent compile-time errors (mpl_assertion_in_line...) with gcc
//#include  <boost/preprocessor/slot/counter.hpp>
//#include BOOST____PP_UPDATE_COUNTER()
//#line BOOST_____PP_COUNTER
BOOST_CLASS_VERSION(model::IVideo, 1)
BOOST_CLASS_EXPORT(model::IVideo)
BOOST_CLASS_TRACKING(model::IVideo, boost::serialization::track_always)

#endif // MODEL_I_VIDEO_H
