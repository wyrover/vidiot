#ifndef MODEL_AUDIO_FILE_H
#define MODEL_AUDIO_FILE_H

#include "File.h"
#include "IAudio.h"

struct ReSampleContext;

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

    AudioFile(wxFileName path);

    virtual AudioFile* clone() const override;

    virtual ~AudioFile();

    //////////////////////////////////////////////////////////////////////////
    // ICONTROL
    //////////////////////////////////////////////////////////////////////////

    virtual void clean() override;

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
    AudioFile(const AudioFile& other);

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    bool mDecodingAudio;
    ReSampleContext* mResampleContext;

    // Do not use the method below for allocating the buffer. That will cause
    // SEGV when compiled with GCC (MingW).
    //int16_t audioDecodeBuffer[AVCODEC_MAX_AUDIO_FRAME_SIZE];
    //int16_t audioResampleBuffer[AVCODEC_MAX_AUDIO_FRAME_SIZE];
    sample* audioDecodeBuffer;
    sample* audioResampleBuffer;
    sample* audioCombineBuffer;

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    void startDecodingAudio(const AudioCompositionParameters& parameters);
    void stopDecodingAudio();

    //////////////////////////////////////////////////////////////////////////
    // FROM FILE
    //////////////////////////////////////////////////////////////////////////

    bool useStream(AVMediaType type) const override;
    void flush() override;

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<<( std::ostream& os, const AudioFile& obj );

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
BOOST_CLASS_VERSION(model::AudioFile, 1)

#endif // MODEL_AUDIO_FILE_H