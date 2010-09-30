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

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

	AudioTrack();

    AudioTrack(const AudioTrack& other);

    virtual AudioTrack* clone();

	virtual ~AudioTrack();

    //////////////////////////////////////////////////////////////////////////
    // HANDLING CLIPS
    //////////////////////////////////////////////////////////////////////////

    void addAudioClip(AudioClipPtr clip);

    //////////////////////////////////////////////////////////////////////////
    // PLAYBACK
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

// Workaround needed to prevent compile-time errors (mpl_assertion_in_line...) with gcc
//#include  <boost/preprocessor/slot/counter.hpp>
//#include BOOST____PP_UPDATE_COUNTER()
//#line BOOST_____PP_COUNTER
BOOST_CLASS_VERSION(model::AudioTrack, 1)
BOOST_CLASS_EXPORT(model::AudioTrack)

#endif // MODEL_AUDIO_TRACK_H
