#ifndef HELPER_TRANSFORM_H
#define HELPER_TRANSFORM_H

#include <boost/shared_ptr.hpp>
#include "UtilEnumSelector.h"

namespace model {
class IClip;
typedef boost::shared_ptr<IClip> IClipPtr;
class VideoTrack;
typedef boost::shared_ptr<VideoTrack> VideoTrackPtr;
class VideoTransition;
typedef boost::shared_ptr<VideoTransition> VideoTransitionPtr;
class AudioTrack;
typedef boost::shared_ptr<AudioTrack> AudioTrackPtr;
class Track;
typedef boost::shared_ptr<Track> TrackPtr;
}

namespace test {

template <class ITEMTYPE>
void ClickOnEnumSelector(EnumSelector<ITEMTYPE>* widget, ITEMTYPE value)
{
    ClickTopLeft(widget);
    Type(WXK_HOME);
    for (int step = 0; step < widget->getIndex(value); ++step)
    {
        Type(WXK_DOWN);
    }
    Type(WXK_RETURN);
    ASSERT_EQUALS(widget->getValue(),value);
    waitForIdle();
}

void ResizeClip(model::IClipPtr clip, wxSize boundingbox);

} // namespace

#endif // HELPER_TRANSFORM_H