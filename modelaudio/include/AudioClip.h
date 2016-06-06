// Copyright 2013-2016 Eric Raijmakers.
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

#include "AudioPeaks.h"
#include "ClipInterval.h"
#include "IAudio.h"

namespace util {
    class SoundTouch;
}

namespace model {

/// Change the volume of the given sample
/// To decrease the volume pass a value < 1.0
/// To increase the volume pass a value > 1.0
/// \param s sample to be adjusted
/// \param volume percentage of original volume
void adjustSampleVolume(const double& volume, sample& s);

class AudioClip
    :   public ClipInterval
    ,   public IAudio
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    AudioClip() = default;

    AudioClip(const AudioFilePtr& clip);

    virtual AudioClip* clone() const override;

    virtual ~AudioClip() = default;

    //////////////////////////////////////////////////////////////////////////
    // ICONTROL
    //////////////////////////////////////////////////////////////////////////

    void clean() override;

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

    AudioPeaks getPeaks(const AudioCompositionParameters& parameters);

protected:

    //////////////////////////////////////////////////////////////////////////
    // COPY CONSTRUCTOR
    //////////////////////////////////////////////////////////////////////////

    /// Copy constructor. Use make_cloned for making deep copies of objects.
    /// \see make_cloned
    AudioClip(const AudioClip& other);

    //////////////////////////////////////////////////////////////////////////
    // KEY FRAMES
    //////////////////////////////////////////////////////////////////////////

    KeyFramePtr interpolate(KeyFramePtr before, KeyFramePtr after, pts positionBefore, pts position, pts positionAfter) const override;

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    pts mProgress = 0; ///< Current render position

    AudioChunkPtr mInputChunk = nullptr;

    std::unique_ptr<util::SoundTouch> mSoundTouch = nullptr;

    /// Cached for performance (stored in the save file to avoid recalculating)
    /// If the clip has a non-default speed and/or volume, the caching is done in
    /// the clip. For default clips (volume/speed) the caching is done 'per file'
    /// in the FileMetaDataCache.
    boost::optional<AudioPeaks> mPeaks = boost::none; 

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

BOOST_CLASS_VERSION(model::AudioClip, 4)
BOOST_CLASS_EXPORT_KEY(model::AudioClip)
