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

    void add(const AudioChunkPtr& chunk);

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

    friend std::ostream& operator<<(std::ostream& os, const AudioComposition& obj);

};

} // namespace
