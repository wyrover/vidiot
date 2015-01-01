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

#ifndef CLIP_CREATOR_H
#define CLIP_CREATOR_H

namespace command {

/// Factory class for creating new clips from a file on disk
class ClipCreator
{
public:

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    /// Create two clips from the given file. Result.first is a video clip, result.second
    /// is an audio clip. If file only contains video data, the audio clip is an empty clip.
    /// If file only contains audio data, the video clip is an empty clip.
    /// \pre file != 0
    /// \pre file->canBeOpened()
    /// \pre file->hasAudio() || file->hasVideo()
    /// \post result.first != 0 && result.second != 0
    /// \post result.first.getLength() == result.second.getLength()
    static std::pair<model::IClipPtr, model::IClipPtr> makeClips(model::FilePtr file);

};

} // namespace

#endif