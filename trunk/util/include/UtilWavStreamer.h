// Copyright 2014-2015 Eric Raijmakers.
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

namespace util {

// Create wav of data being processed, without knowing the resulting data
// size beforehand. Upon desctruction, the proper data size is written to
//  to the file.
//
// Example:
//     Initialization:    util::WavStreamer wavStreamer("d:\\out.wav", 44100, 2);
//     Add chunk of data: wavStreamer.writeData(samples, samples_count);
class WavStreamer
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    explicit WavStreamer(std::string filename, int sampleRate, short channels);
    WavStreamer(const WavStreamer&) = delete;
    WavStreamer& operator=(const WavStreamer&) = delete;
    ~WavStreamer();

    //////////////////////////////////////////////////////////////////////////
    // WRITING DATA
    //////////////////////////////////////////////////////////////////////////

    void writeData(const sample* data, size_t size);

    template <typename T>
    void write(std::ofstream& stream, const T& t)
    {
        stream.write((const char*)&t, sizeof(T));
    }

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    std::string mFileName;
    boost::shared_ptr<std::ofstream> mStream;
    size_t mSize;
};

}

