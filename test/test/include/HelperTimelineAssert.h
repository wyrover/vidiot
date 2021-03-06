// Copyright 2013-2016 Eric Raijmakers.
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

#pragma once

namespace test {

/// Assert that there are no more transitions in the given track
void AssertNoTransitionsInVideoTrack(int trackindex = 0);
#define ASSERT_NO_TRANSITIONS_IN_VIDEO_TRACK AssertNoTransitionsInVideoTrack

/// Assert for the count of the selected clips.
/// Named such for readibility of test cases.
/// Note that the current selected clips (getSelectedClipsCount())
/// are compared to size * 2 (to avoid having to duplicate for
/// the combination of audio and video clips throughout the tests).
void AssertSelectionSize(int size);
#define ASSERT_SELECTION_SIZE AssertSelectionSize

void AssertSelection(model::IClips expected);
#define ASSERT_SELECTION AssertSelection

//////////////////////////////////////////////////////////////////////////
// CLIPTYPEASSERTER
//////////////////////////////////////////////////////////////////////////

struct ClipTypeAsserter
{
    ClipTypeAsserter& CLIPTYPEASSERTER_A;   ///< Helper, in order to be able to compile the code (TYPEASSERTER_* macros)
    ClipTypeAsserter& CLIPTYPEASSERTER_B;   ///< Helper, in order to be able to compile the code (TYPEASSERTER_* macros)

    ClipTypeAsserter(bool video, int TrackNumber, int ClipNumber)
        :   CLIPTYPEASSERTER_A(*this)
        ,   CLIPTYPEASSERTER_B(*this)
        ,   mTrackNumber(TrackNumber)
        ,   mClipNumber(ClipNumber)
        ,   mVideo(video)
    {
    }

    ClipTypeAsserter(const ClipTypeAsserter& other) = delete;

    virtual ~ClipTypeAsserter();

    template<class type>
    ClipTypeAsserter& AssertClipType()
    {
        bool ok = mVideo ? VideoClip(mTrackNumber,mClipNumber)->isA<type>() : AudioClip(mTrackNumber,mClipNumber)->isA<type>();
        if (!ok)
        {
            ClipTypeMismatch(typeid(type));
        }
        mClipNumber++;
        return *this;
    }

    void ClipTypeMismatch(const std::type_info& expectedtype);

    int mTrackNumber;
    int mClipNumber;
    bool mVideo;
};

#define CLIPTYPEASSERTER_A(type) CLIPTYPEASSERTER_OP(type, B)
#define CLIPTYPEASSERTER_B(type) CLIPTYPEASSERTER_OP(type, A)
#define CLIPTYPEASSERTER_OP(type, next) CLIPTYPEASSERTER_A.AssertClipType<model::type>().CLIPTYPEASSERTER_ ## next
#define ASSERT_VIDEOTRACK(TrackNumber,ClipNumber) ClipTypeAsserter(true, TrackNumber,ClipNumber).CLIPTYPEASSERTER_A
#define ASSERT_AUDIOTRACK(TrackNumber,ClipNumber) ClipTypeAsserter(false,TrackNumber,ClipNumber).CLIPTYPEASSERTER_A
#define ASSERT_VIDEOTRACK2 ASSERT_VIDEOTRACK(2,0)
#define ASSERT_VIDEOTRACK1 ASSERT_VIDEOTRACK(1,0)
#define ASSERT_VIDEOTRACK0 ASSERT_VIDEOTRACK(0,0)
#define ASSERT_AUDIOTRACK0 ASSERT_AUDIOTRACK(0,0)
#define ASSERT_AUDIOTRACK1 ASSERT_AUDIOTRACK(1,0)
#define ASSERT_AUDIOTRACK2 ASSERT_AUDIOTRACK(2,0)

#define ASSERT_VIDEOTRACKS(Count) ASSERT_EQUALS(NumberOfVideoTracks(), Count)
#define ASSERT_AUDIOTRACKS(Count) ASSERT_EQUALS(NumberOfAudioTracks(), Count)

#define ASSERT_VIDEOTRACK3SIZE(Count) ASSERT_EQUALS(VideoTrack(3)->getClips().size(), Count)
#define ASSERT_VIDEOTRACK2SIZE(Count) ASSERT_EQUALS(VideoTrack(2)->getClips().size(), Count)
#define ASSERT_VIDEOTRACK1SIZE(Count) ASSERT_EQUALS(VideoTrack(1)->getClips().size(), Count)
#define ASSERT_VIDEOTRACK0SIZE(Count) ASSERT_EQUALS(VideoTrack(0)->getClips().size(), Count)
#define ASSERT_AUDIOTRACK0SIZE(Count) ASSERT_EQUALS(AudioTrack(0)->getClips().size(), Count)
#define ASSERT_AUDIOTRACK1SIZE(Count) ASSERT_EQUALS(AudioTrack(1)->getClips().size(), Count)
#define ASSERT_AUDIOTRACK2SIZE(Count) ASSERT_EQUALS(AudioTrack(2)->getClips().size(), Count)
#define ASSERT_AUDIOTRACK3SIZE(Count) ASSERT_EQUALS(AudioTrack(3)->getClips().size(), Count)

} // namespace
