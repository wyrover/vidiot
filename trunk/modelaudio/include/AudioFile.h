// Copyright 2013,2014 Eric Raijmakers.
//
// This file is part of Vidiot.
//
// Vidiot is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Vidiot is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Vidiot. If not, see <http://www.gnu.org/licenses/>.

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
BOOST_CLASS_EXPORT_KEY(model::AudioFile)

#endif // MODEL_AUDIO_FILE_H