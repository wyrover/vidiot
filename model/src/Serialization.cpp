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

#include "Serialization.h"

#include "AudioClip.h"
#include "AudioCodec.h"
#include "AudioCodecParameter.h"
#include "AudioFile.h"
#include "AudioTrack.h"
#include "AudioTransitionFactory.h"
#include "AutoFolder.h"
#include "Clip.h"
#include "EmptyClip.h"
#include "File.h"
#include "Folder.h"
#include "ImageClip.h"
#include "ImageFile.h"
#include "OutputFormat.h"
#include "Properties.h"
#include "Render.h"
#include "Sequence.h"
#include "Track.h"
#include "Transition.h"
#include "VideoClip.h"
#include "VideoCodec.h"
#include "VideoCodecParameter.h"
#include "VideoFile.h"
#include "VideoTrack.h"
#include "VideoTransition.h"
#include "VideoTransitionFactory.h"

namespace model {

template <class Archive>
void registerClasses(Archive& ar)
{
    ar.template register_type<AudioClip>();
    ar.template register_type<AudioFile>();
    ar.template register_type<AudioTrack>();
    ar.template register_type<AutoFolder>();
    ar.template register_type<EmptyClip>();
    ar.template register_type<File>();
    ar.template register_type<Folder>();
    ar.template register_type<ImageClip>();
    ar.template register_type<ImageFile>();
    ar.template register_type<Properties>();
    ar.template register_type<render::AudioCodec>();
    ar.template register_type<render::OutputFormat>();
    ar.template register_type<render::Render>();
    ar.template register_type<render::VideoCodec>();
    ar.template register_type<Sequence>();
    ar.template register_type<Track>();
    ar.template register_type<Transition>();
    ar.template register_type<VideoClip>();
    ar.template register_type<VideoFile>();
    ar.template register_type<VideoTrack>();
    model::audio::AudioTransitionFactory::get().registerTypesForSerialization(ar);
    model::video::VideoTransitionFactory::get().registerTypesForSerialization(ar);
    render::registerVideoCodecParameterTypesForSerialization(ar);
    render::registerAudioCodecParameterTypesForSerializaton(ar);
}

template void registerClasses<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar);
template void registerClasses<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar);

} //namespace