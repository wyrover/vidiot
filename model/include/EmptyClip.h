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

    EmptyClip(int length);

	virtual ~EmptyClip();

    //////////////////////////////////////////////////////////////////////////
    // IAUDIO
    //////////////////////////////////////////////////////////////////////////

    virtual AudioChunkPtr getNextAudio(int audioRate, int nAudioChannels);

    //////////////////////////////////////////////////////////////////////////
    // IVIDEO
    //////////////////////////////////////////////////////////////////////////

    virtual VideoFramePtr getNextVideo(int requestedWidth, int requestedHeight, bool alpha = true);

private:
    
    //////////////////////////////////////////////////////////////////////////
    // SERIALIZATION 
    //////////////////////////////////////////////////////////////////////////

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);
};

} // namespace

BOOST_CLASS_VERSION(model::EmptyClip, 1)
BOOST_CLASS_EXPORT(model::EmptyClip)

#endif // MODEL_EMPTY_CLIP_H
