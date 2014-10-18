// Copyright 2013,2014 Eric Raijmakers.
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
#include "WximageClip.h"
#include "WximageFile.h"

namespace command {

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

// static
std::pair<model::IClipPtr, model::IClipPtr> ClipCreator::makeClips(model::FilePtr file)
{
    ASSERT(file);
    ASSERT(file->canBeOpened())(file);
    ASSERT(file->hasVideo() || file->hasAudio())(file);

    pts length = file->getLength();
    ASSERT_MORE_THAN_ZERO(length);

    model::IClipPtr videoClip;
    model::IClipPtr audioClip;

    // Create video and/or audio clips
    if (file->hasVideo())
    {
        if (length == 1)
        {
            if (file->isWxImage())
            {
                videoClip = boost::make_shared<model::WximageClip>(boost::make_shared<model::WximageFile>(file->getPath()));
            }
            else
            {
                videoClip = boost::make_shared<model::ImageClip>(boost::make_shared<model::ImageFile>(file->getPath()));
            }
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
    
    // Link the clips if both audio and video data is available
    if (videoClip != nullptr && audioClip != nullptr)
    {
        videoClip->setLink(audioClip);
        audioClip->setLink(videoClip);
    }

    // Create emptyclips for the 'nonexistent' data.
    // Note: Use the length of the other clip. This is required to ensure that the alignment of clips is correct.
    //       For instance, for images the file length == 1, but the clip length equals the default still image length (typically > 1).
    if (videoClip == nullptr)
    {
        videoClip = boost::make_shared<model::EmptyClip>(audioClip->getLength());
    }
    if (audioClip == nullptr)
    {
        audioClip = boost::make_shared<model::EmptyClip>(videoClip->getLength());
    }

    ASSERT_NONZERO(videoClip);
    ASSERT_NONZERO(audioClip);
    ASSERT_EQUALS(videoClip->getLength(),audioClip->getLength());
    return std::make_pair(videoClip,audioClip);
}

} // namespace