#ifndef CROSSFADE_H
#define CROSSFADE_H

#include "VideoTransition.h"

namespace model { namespace transition {

class CrossFade
    :   public VideoTransition
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    CrossFade();

    CrossFade(IClipPtr left, pts nFramesLeft, IClipPtr right, pts nFramesRight);

    virtual CrossFade* clone();

    virtual ~CrossFade();

    //////////////////////////////////////////////////////////////////////////
    // VIDEOTRANSITION
    //////////////////////////////////////////////////////////////////////////

    VideoFramePtr getVideo(pts position, int requestedWidth, int requestedHeight, bool alpha = true);

protected:

    //////////////////////////////////////////////////////////////////////////
    // COPY CONSTRUCTOR
    //////////////////////////////////////////////////////////////////////////

    /// Copy constructor. Use make_cloned for making deep copies of objects.
    /// \see make_cloned
    CrossFade(const CrossFade& other);

private:

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<<( std::ostream& os, const CrossFade& obj );

    //////////////////////////////////////////////////////////////////////////
    // SERIALIZATION
    //////////////////////////////////////////////////////////////////////////

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);
};

}} // namespace

// Workaround needed to prevent compile-time errors (mpl_assertion_in_line...) with gcc
#include  <boost/preprocessor/slot/counter.hpp>
#include BOOST_PP_UPDATE_COUNTER()
#line BOOST_PP_COUNTER
BOOST_CLASS_VERSION(model::transition::CrossFade, 1)

#endif // CROSSFADE_H
