#include "TrackCreator.h"

#include "ClipCreator.h"
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

TrackCreator::TrackCreator(model::NodePtrs assets)
:   mAssets(assets)
,   mVideo(boost::make_shared<model::VideoTrack>())
,   mAudio(boost::make_shared<model::AudioTrack>())
{
    BOOST_FOREACH( model::NodePtr asset, mAssets )
    {
        model::FilePtr file = boost::dynamic_pointer_cast<model::File>(asset);
        if (file && file->canBeOpened())
        {
            std::pair<model::IClipPtr,model::IClipPtr> videoClip_audioClip = ClipCreator::makeClips(file);
            mVideo->addClips(boost::assign::list_of(videoClip_audioClip.first));
            mAudio->addClips(boost::assign::list_of(videoClip_audioClip.second));
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