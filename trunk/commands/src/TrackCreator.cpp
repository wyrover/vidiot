#include "TrackCreator.h"

#include <boost/make_shared.hpp>
#include <boost/foreach.hpp>
#include <boost/assign/list_of.hpp>
#include "VideoTrack.h"
#include "AudioTrack.h"
#include "VideoFile.h"
#include "VideoClip.h"
#include "AudioClip.h"
#include "AudioFile.h"
#include "AProjectViewNode.h"

namespace command {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

TrackCreator::TrackCreator(model::ProjectViewPtrs assets)
:   mAssets(assets)
,   mVideo(boost::make_shared<model::VideoTrack>())
,   mAudio(boost::make_shared<model::AudioTrack>())
{
    BOOST_FOREACH( model::ProjectViewPtr asset, mAssets )
    {
        model::FilePtr file = boost::dynamic_pointer_cast<model::File>(asset);
        if (file)
        {
            VAR_DEBUG(file);
            model::VideoFilePtr videoFile = boost::make_shared<model::VideoFile>(file->getPath());
            model::AudioFilePtr audioFile = boost::make_shared<model::AudioFile>(file->getPath());
            // todo hasvideo and has audio. If not, use emptyclip in other track
            model::VideoClipPtr videoClip = boost::make_shared<model::VideoClip>(videoFile);
            model::AudioClipPtr audioClip = boost::make_shared<model::AudioClip>(audioFile);
            videoClip->setLink(audioClip);
            audioClip->setLink(videoClip);
            mVideo->addClips(boost::assign::list_of(videoClip));
            mAudio->addClips(boost::assign::list_of(audioClip));
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
