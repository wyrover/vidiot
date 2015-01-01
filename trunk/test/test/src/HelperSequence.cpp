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

#include "Test.h"

namespace test {

void ExtendSequenceWithRepeatedClips( model::SequencePtr sequence, model::IPaths files, int nRepeat, bool atBegin )
{
    util::thread::RunInMainAndWait([sequence,files,nRepeat, atBegin]()
    {
        model::TrackPtr videoTrack = sequence->getVideoTrack(0);
        model::TrackPtr audioTrack = sequence->getAudioTrack(0);
        ASSERT(sequence);
        ASSERT(videoTrack);
        ASSERT(audioTrack);
        ASSERT_IMPLIES(atBegin, !videoTrack->getClips().empty());
        ASSERT_IMPLIES(atBegin, !audioTrack->getClips().empty());

        for (int i = 0; i < nRepeat; ++i)
        {
            for ( model::IPathPtr path : files )
            {
                model::FilePtr file = boost::make_shared<model::File>(path->getPath());
                std::pair<model::IClipPtr,model::IClipPtr> videoClip_audioClip = command::ClipCreator::makeClips(file);
                videoTrack->addClips(boost::assign::list_of(videoClip_audioClip.first), atBegin ? videoTrack->getClips().front() : model::IClipPtr() );
                audioTrack->addClips(boost::assign::list_of(videoClip_audioClip.second), atBegin ? audioTrack->getClips().front() : model::IClipPtr());
            }
        }
    });
}

void ExtendSequenceWithRepeatedClipsAtBegin(model::SequencePtr sequence, model::IPaths files, int nRepeat)
{
    ExtendSequenceWithRepeatedClips(sequence,files,nRepeat,true);
}

void ExtendSequenceWithEmptyClipAtBegin(model::SequencePtr sequence, milliseconds time)
{

    for (model::TrackPtr track : sequence->getTracks())
    {
        ASSERT_NONZERO(track->getClips().size());
        model::IClipPtr clip = boost::make_shared<model::EmptyClip>(model::Convert::timeToPts(time));
        track->addClips( boost::assign::list_of(clip), track->getClips().front() );
    }
}

void ExtendSequenceWithStillImage( model::SequencePtr sequence )
{
    util::thread::RunInMainAndWait([sequence]()
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
    ASSERT(sequence);
    util::thread::RunInMainAndWait([sequence]()
    {
        MakeTrackEmpty(sequence->getVideoTrack(0));
        MakeTrackEmpty(sequence->getAudioTrack(0));
    });
}


void ExtendTrack(model::TrackPtr track, model::IPaths files, int nRepeat)
{
    for (int i = 0; i < nRepeat; ++i)
    {
        for (model::IPathPtr path : files)
        {
            model::FilePtr file = boost::make_shared<model::File>(path->getPath());
            std::pair<model::IClipPtr, model::IClipPtr> videoClip_audioClip = command::ClipCreator::makeClips(file);
            if (track->isA<model::VideoTrack>())
            {
                track->addClips(boost::assign::list_of(videoClip_audioClip.first));
            }
            else
            {
                track->addClips(boost::assign::list_of(videoClip_audioClip.second));
            }
        }
    }
}

void MakeTrackEmpty(model::TrackPtr track)
{
    ASSERT(track);
    util::thread::RunInMainAndWait([track]()
    {
        track->removeClips(track->getClips());
    });
    ASSERT_ZERO(track->getClips().size());
}

} // namespace