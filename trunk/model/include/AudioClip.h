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
    // IAUDIO
    //////////////////////////////////////////////////////////////////////////

    virtual AudioChunkPtr getNextAudio(int audioRate, int nAudioChannels);

private:
    
    //////////////////////////////////////////////////////////////////////////
    // SERIALIZATION 
    //////////////////////////////////////////////////////////////////////////

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);
};

} // namespace

BOOST_CLASS_VERSION(model::AudioClip, 1)
BOOST_CLASS_EXPORT(model::AudioClip)

#endif // MODEL_AUDIO_CLIP_H
