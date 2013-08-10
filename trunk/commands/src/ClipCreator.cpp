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

#include "ClipCreator.h"

#include "AudioClip.h"
#include "AudioFile.h"
#include "AudioTrack.h"
#include "EmptyClip.h"
#include "ImageClip.h"
#include "ImageFile.h"
#include "VideoClip.h"
#include "VideoFile.h"
#include "VideoTrack.h"

namespace command {

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

// static
std::pair<model::IClipPtr, model::IClipPtr> ClipCreator::makeClips(model::FilePtr file)
{
    ASSERT(file);
    ASSERT(file->canBeOpened())(file);

    pts length = file->getLength();

    model::IClipPtr videoClip = boost::make_shared<model::EmptyClip>(length);
    model::IClipPtr audioClip = boost::make_shared<model::EmptyClip>(length);

    if (file->hasVideo())
    {
        if (length == 1)
        {
            videoClip = boost::make_shared<model::ImageClip>(boost::make_shared<model::ImageFile>(file->getPath()));
        }
        else
        {
            videoClip = boost::make_shared<model::VideoClip>(boost::make_shared<model::VideoFile>(file->getPath()));
        }
    }
    if (file->hasAudio())
    {
        audioClip = boost::make_shared<model::AudioClip>(boost::make_shared<model::AudioFile>(file->getPath()));
    }
    if (file->hasVideo() && file->hasAudio())
    {
        videoClip->setLink(audioClip);
        audioClip->setLink(videoClip);
    }
    return std::make_pair(videoClip,audioClip);
}

} // namespace