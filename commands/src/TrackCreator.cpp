#include "TrackCreator.h"

#include <boost/make_shared.hpp>
#include <boost/foreach.hpp>
#include <boost/assign/list_of.hpp>
#include "AProjectViewNode.h"
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
            model::IClipPtr videoClip;
            model::IClipPtr audioClip;

            pts length = file->getLength();

            if (file->hasVideo())
            {
                model::VideoFilePtr videoFile = boost::make_shared<model::VideoFile>(file->getPath());
                videoClip = model::IClip::make(boost::make_shared<model::VideoClip>(videoFile));
            }
            else
            {
                videoClip = model::IClip::make(boost::make_shared<model::EmptyClip>(length));
            }
            if (file->hasAudio())
            {
                model::AudioFilePtr audioFile = boost::make_shared<model::AudioFile>(file->getPath());
                audioClip = model::IClip::make(boost::make_shared<model::AudioClip>(audioFile));
            }
            else
            {
                audioClip = model::IClip::make(boost::make_shared<model::EmptyClip>(length));
            }

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
