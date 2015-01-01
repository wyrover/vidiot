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

#ifndef EMPTY_CHUNK_H
#define EMPTY_CHUNK_H

#include "AudioChunk.h"

namespace model {

class EmptyChunk
    :   public AudioChunk
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    EmptyChunk(int nChannels, samplecount nSamples);

    virtual ~EmptyChunk();

    //////////////////////////////////////////////////////////////////////////
    // DATA ACCESS
    //////////////////////////////////////////////////////////////////////////

    /// Override from AudioChunk. When this method is called for the first time,
    /// then the zeroed buffer is generated.
    sample* getUnreadSamples() override;

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    bool mInitialized;
};

} // namespace

#endif
