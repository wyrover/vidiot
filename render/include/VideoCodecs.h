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

namespace model { namespace render {

typedef std::map<AVCodecID,VideoCodecPtr> VideoCodecMap;

class VideoCodecs
{
public:
    static void initialize();
    static void add(const wxString& name, const VideoCodec& codec);

    static std::map<int,wxString> mapToName;

    static VideoCodecPtr getDefault();
    static std::vector<AVCodecID> all();

    /// \return 0 if a codec with the given id was not found
    /// This method finds the given codec in the registered list of codecs
    static VideoCodecPtr find(const AVCodecID& id);
private:
    static VideoCodecMap sVideoCodecs;
};

}} // namespace
