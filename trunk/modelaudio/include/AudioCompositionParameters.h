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

#ifndef AUDIO_FRAME_COMPOSITION_PARAMETERS_H
#define AUDIO_FRAME_COMPOSITION_PARAMETERS_H

#include "IAudio.h"
#include "UtilInt.h"

namespace model {

class AudioCompositionParameters
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    AudioCompositionParameters();
    AudioCompositionParameters(const AudioCompositionParameters& other);

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    AudioCompositionParameters& setSampleRate(int audiorate);
    int getSampleRate() const;

    AudioCompositionParameters& setNrChannels(int nChannels);
    int getNrChannels() const;

    AudioCompositionParameters& setPts(pts position);
    AudioCompositionParameters& adjustPts(pts adjustment);
    bool hasPts() const;
    pts getPts() const;

    AudioCompositionParameters& determineChunkSize();
    samplecount getChunkSize() const;

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    int mSampleRate;    ///< The rate (samples per second) at which audio will be played (44100/48000/etc.)
    int mNrChannels;    ///< Number of audio channels (speakers)
    boost::optional<pts> mPts;
    boost::optional<samplecount> mChunkSize;

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<<(std::ostream& os, const AudioCompositionParameters& obj);

};

} // namespace

#endif
