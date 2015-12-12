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

#include "DetailsClip.h"

#include "AudioClip.h"
#include "AudioClipEvent.h"
#include "TransitionParameter.h"
#include "UtilClone.h"
#include "VideoClipEvent.h"
#include "VideoClipKeyFrame.h"

namespace gui { namespace timeline {

DetailsClip::ClonesContainer::ClonesContainer(DetailsClip* details, model::IClipPtr clip)
    : mDetails(details)
{
    Clip = make_cloned<model::IClip>(clip);
    Link = clip->getLink() ? make_cloned<model::IClip>(clip->getLink()) : nullptr;

    if (clip->isA<model::VideoClip>())
    {
        Video = boost::dynamic_pointer_cast<model::VideoClip>(Clip);
        Audio = boost::dynamic_pointer_cast<model::AudioClip>(Link);
    }
    else if (clip->isA<model::AudioClip>())
    {
        Audio = boost::dynamic_pointer_cast<model::AudioClip>(Clip);
        Video = boost::dynamic_pointer_cast<model::VideoClip>(Link);
    }
    if (Clip && Link)
    {
        Clip->setLink(Link);
        Link->setLink(Clip);
    }

    if (Audio)
    {
        // todo do this via the cleanup class
        Audio->Bind(model::EVENT_CHANGE_AUDIOCLIP_VOLUME, &DetailsClip::onVolumeChanged, mDetails);
    }
};
    
DetailsClip::ClonesContainer::~ClonesContainer()
{
    if (Audio)
    {
        // todo do this via the cleanup class
        Audio->Unbind(model::EVENT_CHANGE_AUDIOCLIP_VOLUME, &DetailsClip::onVolumeChanged, mDetails);
    }
    Clip.reset();
    Link.reset();
    Video.reset();
    Audio.reset();
};

}} // namespace
