#include "ClipCreator.h"

#include "AudioClip.h"
#include "AudioFile.h"
#include "AudioTrack.h"
#include "EmptyClip.h"
#include "VideoClip.h"
#include "VideoFile.h"
#include "VideoTrack.h"

namespace command {

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

// static
std::pair<model::IClipPtr, model::IClipPtr> ClipCreator::makeClips(model::FilePtr file)
{
    ASSERT(file);
    ASSERT(file->canBeOpened())(file);

    pts length = file->getLength();

    model::IClipPtr videoClip = boost::make_shared<model::EmptyClip>(length);
    model::IClipPtr audioClip = boost::make_shared<model::EmptyClip>(length);

    if (file->hasVideo())
    {
        if (length == 1)
        {

            // todo if length == 1 then make a still image file....
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
    if (file->hasVideo() && file->hasAudio())
    {
        videoClip->setLink(audioClip);
        audioClip->setLink(videoClip);
    }
    return std::make_pair(videoClip,audioClip);
}

} // namespace