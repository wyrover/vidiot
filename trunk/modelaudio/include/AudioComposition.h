#ifndef AUDIO_CHUNK_COMPOSITION_H
#define AUDIO_CHUNK_COMPOSITION_H

#include "AudioCompositionParameters.h"

namespace model {

class AudioCompositionParameters;

class AudioComposition
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    AudioComposition(const model::AudioCompositionParameters& parameters);
    AudioComposition(const AudioComposition& other);
    virtual ~AudioComposition();

    //////////////////////////////////////////////////////////////////////////
    // COMPOSITION
    //////////////////////////////////////////////////////////////////////////

    void add(AudioChunkPtr chunk);
    void replace(AudioChunkPtr oldChunk, AudioChunkPtr newChunk);

    /// Render the composition
    /// \return composition of all input chunks.
    /// \note may return '0' to indicate that the composition is completely empty.
    /// \note the pts position value of the returned chunk is always 0
    AudioChunkPtr generate();

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    AudioCompositionParameters getParameters() const;

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    AudioCompositionParameters mParameters;
    AudioChunks mChunks;

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<<( std::ostream& os, const AudioComposition& obj );

};

} // namespace

#endif // AUDIO_CHUNK_COMPOSITION_H