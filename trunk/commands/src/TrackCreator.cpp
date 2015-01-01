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

#include "TrackCreator.h"

#include "ClipCreator.h"
#include "Config.h"
#include "Node.h"
#include "AudioClip.h"
#include "AudioFile.h"
#include "AudioTrack.h"
#include "EmptyClip.h"
#include "VideoClip.h"
#include "VideoFile.h"
#include "VideoTrack.h"

namespace command {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

TrackCreator::TrackCreator(const model::NodePtrs& assets)
:   mAssets(assets)
,   mVideo(boost::make_shared<model::VideoTrack>())
,   mAudio(boost::make_shared<model::AudioTrack>())
{
    wxString previousPrefix("");
    int prefixLength = Config::ReadLong(Config::sPathMakeSequencePrefixLength);
    int emptyLength = Config::ReadLong(Config::sPathMakeSequenceEmptyClipLength);

    for ( model::NodePtr asset : mAssets )
    {
        model::FilePtr file = boost::dynamic_pointer_cast<model::File>(asset);
        if (file && file->canBeOpened())
        {
            if (emptyLength > 0)
            {
                wxString newPrefix(file->getName().Left(prefixLength));
                if (!previousPrefix.IsEmpty() &&
                    !previousPrefix.IsSameAs(newPrefix))
                {
                    mVideo->addClips(boost::assign::list_of(boost::make_shared<model::EmptyClip>(emptyLength)));
                    mAudio->addClips(boost::assign::list_of(boost::make_shared<model::EmptyClip>(emptyLength)));
                }
            }

            std::pair<model::IClipPtr,model::IClipPtr> videoClip_audioClip = ClipCreator::makeClips(file);

            mVideo->addClips(boost::assign::list_of(videoClip_audioClip.first));
            mAudio->addClips(boost::assign::list_of(videoClip_audioClip.second));

            previousPrefix = file->getName().Left(prefixLength);
        }
    }
}

TrackCreator::~TrackCreator()
{
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

model::TrackPtr TrackCreator::getVideoTrack()
{
    return mVideo;
}

model::TrackPtr TrackCreator::getAudioTrack()
{
    return mAudio;
}

} // namespace