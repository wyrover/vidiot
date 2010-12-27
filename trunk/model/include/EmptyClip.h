#ifndef MODEL_EMPTY_CLIP_H
#define MODEL_EMPTY_CLIP_H

#include "Clip.h"
#include "IAudio.h"
#include "IVideo.h"

namespace model {

class EmptyClip
    :   public Clip
    ,   public IAudio
    ,   public IVideo
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    EmptyClip();

    EmptyClip(pts length);

    virtual EmptyClip* clone();

    virtual ~EmptyClip();

    //////////////////////////////////////////////////////////////////////////
    // IAUDIO
    //////////////////////////////////////////////////////////////////////////

    virtual AudioChunkPtr getNextAudio(int audioRate, int nAudioChannels);

    //////////////////////////////////////////////////////////////////////////
    // IVIDEO
    //////////////////////////////////////////////////////////////////////////

    virtual VideoFramePtr getNextVideo(int requestedWidth, int requestedHeight, bool alpha = true);

protected:

    //////////////////////////////////////////////////////////////////////////
    // COPY CONSTRUCTOR
    //////////////////////////////////////////////////////////////////////////

    /// Copy constructor. Use make_cloned for making deep copies of objects.
    /// @see make_cloned
    EmptyClip(const EmptyClip& other);

private:

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
BOOST_CLASS_VERSION(model::EmptyClip, 1)
BOOST_CLASS_EXPORT(model::EmptyClip)

#endif // MODEL_EMPTY_CLIP_H
