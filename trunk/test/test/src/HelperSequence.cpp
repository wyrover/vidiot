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

#include "Test.h"

namespace test {

void ExtendSequenceWithRepeatedClips( model::SequencePtr sequence, model::IPaths files, int nRepeat )
{
    RunInMainAndWait([sequence,files,nRepeat]()
    {
        model::TrackPtr videoTrack = sequence->getVideoTrack(0);
        model::TrackPtr audioTrack = sequence->getAudioTrack(0);
        ASSERT(sequence);
        ASSERT(videoTrack);
        ASSERT(audioTrack);

        for (int i = 0; i < nRepeat; ++i)
        {
            for ( model::IPathPtr path : files )
            {
                model::FilePtr file = boost::make_shared<model::File>(path->getPath());
                std::pair<model::IClipPtr,model::IClipPtr> videoClip_audioClip = command::ClipCreator::makeClips(file);
                videoTrack->addClips(boost::assign::list_of(videoClip_audioClip.first));
                audioTrack->addClips(boost::assign::list_of(videoClip_audioClip.second));
            }
        }
    });
}

void ExtendSequenceWithStillImage( model::SequencePtr sequence )
{
    RunInMainAndWait([sequence]()
    {
        model::TrackPtr videoTrack = sequence->getVideoTrack(0);
        model::TrackPtr audioTrack = sequence->getAudioTrack(0);
        ASSERT(sequence);
        ASSERT(videoTrack);
        ASSERT(audioTrack);

        model::FilePtr file = boost::make_shared<model::File>(getStillImagePath());
        ASSERT(file);
        ASSERT(file->canBeOpened());

        std::pair<model::IClipPtr,model::IClipPtr> videoClip_audioClip = command::ClipCreator::makeClips(file);
        videoTrack->addClips(boost::assign::list_of(videoClip_audioClip.first));
        audioTrack->addClips(boost::assign::list_of(videoClip_audioClip.second));
    });
}

void MakeSequenceEmpty( model::SequencePtr sequence )
{
    RunInMainAndWait([sequence]()
    {
        model::TrackPtr videoTrack = sequence->getVideoTrack(0);
        model::TrackPtr audioTrack = sequence->getAudioTrack(0);
        ASSERT(sequence);
        ASSERT(videoTrack);
        ASSERT(audioTrack);
        videoTrack->removeClips(videoTrack->getClips());
        audioTrack->removeClips(audioTrack->getClips());
        ASSERT_ZERO(videoTrack->getClips().size());
        ASSERT_ZERO(audioTrack->getClips().size());
    });
}

} // namespace