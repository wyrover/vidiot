// Copyright 2013-2015 Eric Raijmakers.
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

#pragma once

#include "ClipInterval.h"
#include "IAudio.h"

namespace model {

class AudioPeaks;

class AudioClip
    :   public ClipInterval
    ,   public IAudio
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    AudioClip();

    AudioClip(const AudioFilePtr& clip);

    virtual AudioClip* clone() const override;

    virtual ~AudioClip();

    //////////////////////////////////////////////////////////////////////////
    // ICONTROL
    //////////////////////////////////////////////////////////////////////////

    virtual void clean() override;

    //////////////////////////////////////////////////////////////////////////
    // ICLIP
    //////////////////////////////////////////////////////////////////////////

    virtual std::ostream& dump(std::ostream& os) const override;
    virtual const char* getType() const override;

    //////////////////////////////////////////////////////////////////////////
    // IAUDIO
    //////////////////////////////////////////////////////////////////////////

    virtual AudioChunkPtr getNextAudio(const AudioCompositionParameters& parameters) override;

    //////////////////////////////////////////////////////////////////////////
    // AUDIOCLIP
    //////////////////////////////////////////////////////////////////////////

    void setVolume(int volume);
    int getVolume() const;
    AudioPeaks getPeaks(const AudioCompositionParameters& parameters);

protected:

    //////////////////////////////////////////////////////////////////////////
    // COPY CONSTRUCTOR
    //////////////////////////////////////////////////////////////////////////

    /// Copy constructor. Use make_cloned for making deep copies of objects.
    /// \see make_cloned
    AudioClip(const AudioClip& other);

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    pts mProgress; ///< Current render position
    int mVolume;

    AudioChunkPtr mInputChunk;

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<<(std::ostream& os, const AudioClip& obj);

    //////////////////////////////////////////////////////////////////////////
    // SERIALIZATION
    //////////////////////////////////////////////////////////////////////////

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);
};

} // namespace

BOOST_CLASS_VERSION(model::AudioClip, 2)
BOOST_CLASS_EXPORT_KEY(model::AudioClip)
