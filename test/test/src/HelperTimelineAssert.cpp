#pragma warning(disable:4996)

#include "HelperTimeline.h"

#include "HelperTimelineAssert.h"
#include "Transition.h"
#include "UtilLog.h"
#include "VideoClip.h"

namespace test {

void ASSERT_NO_TRANSITIONS_IN_VIDEO_TRACK(int trackindex)
{
    for (int i = 0; i < NumberOfVideoClipsInTrack(trackindex); ++i)
    {
        ASSERT(!VideoClip(trackindex,i)->isA<model::Transition>())(i);
    }
}

void ASSERT_SELECTION_SIZE(int size)
{
    ASSERT_EQUALS(getSelectedClipsCount(),2 * size); // * 2 since AudioClips are selected also
}

//////////////////////////////////////////////////////////////////////////
// CLIPTYPEASSERTER
//////////////////////////////////////////////////////////////////////////

ClipTypeAsserter::~ClipTypeAsserter()
{
};

void ClipTypeAsserter::ClipTypeMismatch(const type_info& expectedtype)
{
    int TrackNumber = mTrackNumber;
    int ClipNumber = mClipNumber;
    std::string TrackType = mVideo ? "VIDEO" : "AUDIO";
    auto convert = [](const type_info& info) -> std::string
    {
        std::string theName = info.name();
        return theName.substr(theName.find_last_of(':') + 1);
    };

    std::string Expected = convert(expectedtype);
    std::string Got = convert(typeid(*VideoClip(TrackNumber,mClipNumber)));
    LogVar("Clip type error", __FILE__, __LINE__,__FUNCTION__).LOGVAR_A(TrackType)(TrackNumber)(mClipNumber)(Expected)(Got);
}

} // namespace