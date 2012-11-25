#ifndef MODEL_AUDIO_TRANSITION_H
#define MODEL_AUDIO_TRANSITION_H

#include "Transition.h"
#include "IAudio.h"

namespace model {

class AudioTransition
    :   public Transition
    ,   public IAudio
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    AudioTransition();

    virtual ~AudioTransition();

   //////////////////////////////////////////////////////////////////////////
    // IAUDIO
    //////////////////////////////////////////////////////////////////////////

    virtual AudioChunkPtr getNextAudio(const AudioCompositionParameters& parameters) override;

protected:

    //////////////////////////////////////////////////////////////////////////
    // COPY CONSTRUCTOR
    //////////////////////////////////////////////////////////////////////////

    /// Copy constructor. Use make_cloned for making deep copies of objects.
    /// \see make_cloned
    AudioTransition(const AudioTransition& other);

    //////////////////////////////////////////////////////////////////////////
    // IMPLEMENTATION OF TRANSITION
    //////////////////////////////////////////////////////////////////////////

    samplecount getTotalSamples(const AudioCompositionParameters& parameters) const;

    virtual void reset() = 0; ///< Reset any data that is cached.
    virtual AudioChunkPtr getAudio(samplecount position, IClipPtr leftClip, IClipPtr rightClip, const AudioCompositionParameters& parameters) = 0;

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    samplecount mProgress;  ///< Last rendered position in frames
    IClipPtr mLeftClip;     ///< Clip generating 'left' side. NOTE: Only used for generating frames, not for querying. That should be done by inspecting 'IClip::getPrev'
    IClipPtr mRightClip;    ///< Clip generating 'right' side. NOTE: Only used for generating frames, not for querying. That should be done by inspecting 'IClip::getNext'

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<<( std::ostream& os, const AudioTransition& obj );

    //////////////////////////////////////////////////////////////////////////
    // SERIALIZATION
    //////////////////////////////////////////////////////////////////////////

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);
};

} // namespace

// Workaround needed to prevent compile-time errors (mpl_assertion_in_line...) with gcc
#include  <boost/preprocessor/slot/counter.hpp>
#include BOOST_PP_UPDATE_COUNTER()
#line BOOST_PP_COUNTER
BOOST_CLASS_VERSION(model::AudioTransition, 1);

#endif // MODEL_AUDIO_TRANSITION_H