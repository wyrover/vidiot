// Copyright 2016 Eric Raijmakers.
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

namespace soundtouch {
    class SoundTouch;
}

namespace util {

class SoundTouch
{
public:

    static const int sMinimumSpeed;
    static const int sMaximumSpeed;
    static const int sDefaultSpeed;

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION 
    //////////////////////////////////////////////////////////////////////////

    SoundTouch(int samplerate, int channels, rational64 speed);
    virtual ~SoundTouch();

    //////////////////////////////////////////////////////////////////////////
    // INTERFACE
    //////////////////////////////////////////////////////////////////////////

    void send(model::AudioChunkPtr chunk);
    samplecount receive(model::AudioChunkPtr chunk, samplecount offset, samplecount nSamplesRequired);

    bool isEmpty();
    bool atEnd();

    rational64 getSpeed() const;

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////
    // AUDIO
    //////////////////////////////////////////////////////////////////////////

    std::unique_ptr<soundtouch::SoundTouch> mSoundTouch;
    int mSampleRate = 0;
    int mChannels = 0;
    rational64 mSpeed = 1;
    bool mAtEnd = false;

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////
};

}

