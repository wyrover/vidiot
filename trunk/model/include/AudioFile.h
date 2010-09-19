#ifndef MODEL_AUDIO_FILE_H
#define MODEL_AUDIO_FILE_H

#include <boost/filesystem/path.hpp>
#include "File.h"
#include "IAudio.h"
#include "ModelPtr.h"

namespace model {

class AudioFile 
    :   public File
    ,   public IAudio
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    AudioFile();

	AudioFile(boost::filesystem::path path);

    AudioFile(const AudioFile& other);

    virtual AudioFile* clone();

	virtual ~AudioFile();

    //////////////////////////////////////////////////////////////////////////
    // IAUDIO
    //////////////////////////////////////////////////////////////////////////

    virtual AudioChunkPtr getNextAudio(int audioRate, int nAudioChannels);

private:

    void startDecodingAudio(int audioRate, int nAudioChannels);
    void stopDecodingAudio();

    bool mDecodingAudio;

	//////////////////////////////////////////////////////////////////////////
	// AUDIO
	//////////////////////////////////////////////////////////////////////////

    ReSampleContext*    mResampleContext;

    int16_t audioDecodeBuffer[AVCODEC_MAX_AUDIO_FRAME_SIZE];
    int16_t audioResampleBuffer[AVCODEC_MAX_AUDIO_FRAME_SIZE];

    //////////////////////////////////////////////////////////////////////////
    // SERIALIZATION 
    //////////////////////////////////////////////////////////////////////////

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);
};

} // namespace

BOOST_CLASS_VERSION(model::AudioFile, 1)
BOOST_CLASS_EXPORT(model::AudioFile)

#endif // MODEL_AUDIO_FILE_H
