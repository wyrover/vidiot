#ifndef MODEL_AUDIO_TRACK_H
#define MODEL_AUDIO_TRACK_H

#include "Track.h"
#include "IAudio.h"

namespace model {

class AudioTrack
    :   public Track
    ,   public IAudio
{
public:

	AudioTrack();
	virtual ~AudioTrack();

    void addAudioClip(AudioClipPtr clip);
    void removeAudioClip(AudioClipPtr clip);

    virtual AudioChunkPtr getNextAudio(int audioRate, int nAudioChannels);

    //////////////////////////////////////////////////////////////////////////
    // SERIALIZATION 
    //////////////////////////////////////////////////////////////////////////

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);
};

} // namespace

BOOST_CLASS_VERSION(model::AudioTrack, 1)
BOOST_CLASS_EXPORT(model::AudioTrack)

#endif // MODEL_AUDIO_TRACK_H
