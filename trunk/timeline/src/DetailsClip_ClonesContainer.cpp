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
    else if (clip->isA<model::Transition>())
    {
        Transition = boost::dynamic_pointer_cast<model::Transition>(Clip);
    }

    if (Video)
    {
        Video->Bind(model::EVENT_CHANGE_VIDEOCLIP_OPACITY, &DetailsClip::onOpacityChanged, mDetails);
        Video->Bind(model::EVENT_CHANGE_VIDEOCLIP_SCALING, &DetailsClip::onScalingChanged, mDetails);
        Video->Bind(model::EVENT_CHANGE_VIDEOCLIP_SCALINGFACTOR, &DetailsClip::onScalingFactorChanged, mDetails);
        Video->Bind(model::EVENT_CHANGE_VIDEOCLIP_ROTATION, &DetailsClip::onRotationChanged, mDetails);
        Video->Bind(model::EVENT_CHANGE_VIDEOCLIP_ALIGNMENT, &DetailsClip::onAlignmentChanged, mDetails);
        Video->Bind(model::EVENT_CHANGE_VIDEOCLIP_POSITION, &DetailsClip::onPositionChanged, mDetails);
        Video->Bind(model::EVENT_CHANGE_VIDEOCLIP_MINPOSITION, &DetailsClip::onMinPositionChanged, mDetails);
        Video->Bind(model::EVENT_CHANGE_VIDEOCLIP_MAXPOSITION, &DetailsClip::onMaxPositionChanged, mDetails);
    }
    if (Audio)
    {
        Audio->Bind(model::EVENT_CHANGE_AUDIOCLIP_VOLUME, &DetailsClip::onVolumeChanged, mDetails);
    }
    if (Transition)
    {
        mDetails->setBox(mDetails->mTransitionBoxSizer);
        for (auto id_and_parameter : Transition->getParameters())
        {
            mDetails->addOption(
                id_and_parameter.second->getName(),
                id_and_parameter.second->makeWidget(mDetails));
            id_and_parameter.second->Bind(model::EVENT_TRANSITION_PARAMETER_CHANGED, &DetailsClip::onTransitionParameterChanged, mDetails);
        }
    }
};
    
DetailsClip::ClonesContainer::~ClonesContainer()
{
    if (Video)
    {
        Video->Unbind(model::EVENT_CHANGE_VIDEOCLIP_OPACITY, &DetailsClip::onOpacityChanged, mDetails);
        Video->Unbind(model::EVENT_CHANGE_VIDEOCLIP_SCALING, &DetailsClip::onScalingChanged, mDetails);
        Video->Unbind(model::EVENT_CHANGE_VIDEOCLIP_SCALINGFACTOR, &DetailsClip::onScalingFactorChanged, mDetails);
        Video->Unbind(model::EVENT_CHANGE_VIDEOCLIP_ROTATION, &DetailsClip::onRotationChanged, mDetails);
        Video->Unbind(model::EVENT_CHANGE_VIDEOCLIP_ALIGNMENT, &DetailsClip::onAlignmentChanged, mDetails);
        Video->Unbind(model::EVENT_CHANGE_VIDEOCLIP_POSITION, &DetailsClip::onPositionChanged, mDetails);
        Video->Unbind(model::EVENT_CHANGE_VIDEOCLIP_MINPOSITION, &DetailsClip::onMinPositionChanged, mDetails);
        Video->Unbind(model::EVENT_CHANGE_VIDEOCLIP_MAXPOSITION, &DetailsClip::onMaxPositionChanged, mDetails);
    }
    if (Audio)
    {
        Audio->Unbind(model::EVENT_CHANGE_AUDIOCLIP_VOLUME, &DetailsClip::onVolumeChanged, mDetails);
    }
    if (Transition)
    {
        while (mDetails->mTransitionBoxSizer->GetItemCount() > 0)
        {
            wxWindow* widget = mDetails->mTransitionBoxSizer->GetItem(static_cast<size_t>(0))->GetWindow();
            mDetails->mTransitionBoxSizer->Detach(0);
            if (dynamic_cast<wxStaticText*>(widget) != 0)
            {
                // It's a title. Must be removed also.
                widget->Destroy();
            }
        }
        for (auto id_and_parameter : Transition->getParameters())
        {
            id_and_parameter.second->Unbind(model::EVENT_TRANSITION_PARAMETER_CHANGED, &DetailsClip::onTransitionParameterChanged, mDetails);
            id_and_parameter.second->destroyWidget();
        }
    }
    Clip.reset();
    Link.reset();
    Video.reset();
    Audio.reset();
    Transition.reset();
};

std::tuple<model::VideoClipPtr, model::AudioClipPtr, model::TransitionPtr> getTypedClips(model::IClipPtr clip) // todo make part of model?
{
    model::VideoClipPtr video = nullptr;
    model::AudioClipPtr audio = nullptr;
    model::TransitionPtr transition = nullptr;
    if (clip->isA<model::VideoClip>())
    {
        video = boost::dynamic_pointer_cast<model::VideoClip>(clip);
        audio = boost::dynamic_pointer_cast<model::AudioClip>(clip->getLink());
    }
    else if (clip->isA<model::AudioClip>())
    {
        audio = boost::dynamic_pointer_cast<model::AudioClip>(clip);
        video = boost::dynamic_pointer_cast<model::VideoClip>(clip->getLink());
    }
    else if (clip->isA<model::Transition>())
    {
        transition = boost::dynamic_pointer_cast<model::Transition>(clip);
    }
    return std::make_tuple(video, audio, transition);
}

}} // namespace
