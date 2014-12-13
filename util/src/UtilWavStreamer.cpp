// Copyright 2014 Eric Raijmakers.
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

#include "UtilWavStreamer.h"

#include "stdio.h"

namespace util {

WavStreamer::WavStreamer(std::string filename, int sampleRate, short channels)
    : mFileName(filename)
    , mStream(boost::make_shared<std::ofstream>(filename, std::ios::binary))
    , mSize(0)
{
    mStream->write("RIFF", 4);
    write<int>(*mStream, 36 + 0x00000000);
    mStream->write("WAVE", 4);
    mStream->write("fmt ", 4);
    write<int>(*mStream, 16);
    write<short>(*mStream, 1); // Format
    write<short>(*mStream, channels);                             // Channels
    write<int>(*mStream, sampleRate);                             // Sample Rate
    write<int>(*mStream, sampleRate * channels * sizeof(sample)); // Byterate
    write<short>(*mStream, channels * sizeof(sample));            // Frame size
    write<short>(*mStream, 8 * sizeof(sample));                  // Bits per sample
    mStream->write("data", 4);
    write<int>(*mStream, 0); // Reserved for size of buffer
    // mStream.write((const char*)buf, bufSize);
}

WavStreamer::~WavStreamer()
{
    mStream.reset();
    size_t totalSize = 36 + mSize; // Includes header
    FILE* f = fopen(mFileName.c_str(), "r+b"); // Error checking omitted
    fseek(f, 4, SEEK_SET);
    fwrite(&totalSize, sizeof(size_t), 1, f);
    fseek(f, 40, SEEK_SET);
    fwrite(&mSize, sizeof(size_t), 1, f);
    fclose(f);
}

void WavStreamer::writeData(const sample* data, size_t size)
{
    mSize += size * sizeof(sample);

    mStream->write((const char*)(data), size * sizeof(sample));

    VAR_ERROR(size)(mSize);
}

}