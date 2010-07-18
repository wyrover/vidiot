#ifndef MODEL_EMPTY_FILE_H
#define MODEL_EMPTY_FILE_H

#include "IControl.h"
#include "IAudio.h"
#include "IVideo.h"

namespace model {

class EmptyFile 
    :   public IControl
    ,   public IAudio
    ,   public IVideo
{
public:

    /**
     * Statics. These are merely used as 'signaling' tokens.
     * EmptyAudioChunk indicates silence with a duration of one video frame.
     */
    static AudioChunkPtr EmptyAudioChunk;
    static VideoFramePtr EmptyVideoFrame;

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    EmptyFile();

	EmptyFile(boost::int64_t length);

	virtual ~EmptyFile();

    //////////////////////////////////////////////////////////////////////////
    // ICONTROL
    //////////////////////////////////////////////////////////////////////////

    virtual boost::int64_t getNumberOfFrames();
    virtual void moveTo(boost::int64_t position);

    //////////////////////////////////////////////////////////////////////////
    // IAUDIO
    //////////////////////////////////////////////////////////////////////////

    virtual AudioChunkPtr getNextAudio(int audioRate, int nAudioChannels);

    //////////////////////////////////////////////////////////////////////////
    // IVIDEO
    //////////////////////////////////////////////////////////////////////////

    virtual VideoFramePtr getNextVideo(int requestedWidth, int requestedHeight, bool alpha = true);

private:

    boost::int64_t mLength;
    boost::int64_t mAudioPosition;
    boost::int64_t mVideoPosition;

    //////////////////////////////////////////////////////////////////////////
    // SERIALIZATION 
    //////////////////////////////////////////////////////////////////////////

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);
};

} // namespace

BOOST_CLASS_VERSION(model::EmptyFile, 1)
BOOST_CLASS_EXPORT(model::EmptyFile)

#endif // MODEL_EMPTY_FILE_H
