// Copyright 2013 Eric Raijmakers.
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

#ifndef UTIL_LOG_AVCODEC_H
#define UTIL_LOG_AVCODEC_H

struct AVRational;
struct AVCodecContext;
struct AVFormatContext;
struct AVStream;
struct AVPacket;
struct AVOutputFormat;
enum CodecID;

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<< (std::ostream& os, const PixelFormat& obj);
std::ostream& operator<< (std::ostream& os, const AVRational& obj);
std::ostream& operator<< (std::ostream& os, const AVCodec* obj);
std::ostream& operator<< (std::ostream& os, const AVCodecContext* obj);
std::ostream& operator<< (std::ostream& os, const AVFormatContext* obj);
std::ostream& operator<< (std::ostream& os, const AVStream* obj);
std::ostream& operator<< (std::ostream& os, const AVPacket* obj);
std::ostream& operator<< (std::ostream& os, const CodecID& obj);
std::ostream& operator<< (std::ostream& os, const AVOutputFormat* obj);
std::ostream& operator<< (std::ostream& os, const AVSampleFormat& obj);

typedef std::pair<wxString, int> LevelString;

std::string avcodecErrorString(int errorcode);

#endif //UTIL_LOG_AVCODEC_H