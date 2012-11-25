#ifndef AUDIO_TRANSITION_CROSSFADE_H
#define AUDIO_TRANSITION_CROSSFADE_H

#include "AudioTransition.h"

namespace model { namespace audio { namespace transition {

struct Cache;

class CrossFade
    :   public AudioTransition
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    CrossFade();
    virtual CrossFade* clone() const;
    virtual ~CrossFade();

    //////////////////////////////////////////////////////////////////////////
    // AUDIOTRANSITION
    //////////////////////////////////////////////////////////////////////////

    virtual void reset();
    virtual AudioChunkPtr getAudio(samplecount position, IClipPtr leftClip, IClipPtr rightClip, const AudioCompositionParameters& parameters) override;

protected:

    //////////////////////////////////////////////////////////////////////////
    // COPY CONSTRUCTOR
    //////////////////////////////////////////////////////////////////////////

    /// Copy constructor. Use make_cloned for making deep copies of objects.
    /// \see make_cloned
    CrossFade(const CrossFade& other);

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    boost::shared_ptr<Cache> mCache;

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

}}} // namespace

// Workaround needed to prevent compile-time errors (mpl_assertion_in_line...) with gcc
#include  <boost/preprocessor/slot/counter.hpp>
#include BOOST_PP_UPDATE_COUNTER()
#line BOOST_PP_COUNTER
BOOST_CLASS_VERSION(model::audio::transition::CrossFade, 1)

#endif // AUDIO_TRANSITION_CROSSFADE_H