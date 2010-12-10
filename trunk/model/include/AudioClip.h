#ifndef MODEL_AUDIO_CLIP_H
#define MODEL_AUDIO_CLIP_H

#include "Clip.h"
#include "IAudio.h"

namespace model {

class AudioClip
    :   public Clip
    ,   public IAudio
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    AudioClip();

    AudioClip(AudioFilePtr clip);

    AudioClip(const AudioClip& other);

    virtual AudioClip* clone();

    virtual ~AudioClip();

    //////////////////////////////////////////////////////////////////////////
    // ICONTROL
    //////////////////////////////////////////////////////////////////////////

    virtual void moveTo(pts position);

    //////////////////////////////////////////////////////////////////////////
    // IAUDIO
    //////////////////////////////////////////////////////////////////////////

    virtual AudioChunkPtr getNextAudio(int audioRate, int nAudioChannels);

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    pts mLastSetPosition;                       ///< Last position that was explicitly set
    boost::optional<unsigned int> mProgress;    ///< Current render position in samples

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
BOOST_CLASS_VERSION(model::AudioClip, 1)
BOOST_CLASS_EXPORT(model::AudioClip)

#endif // MODEL_AUDIO_CLIP_H
