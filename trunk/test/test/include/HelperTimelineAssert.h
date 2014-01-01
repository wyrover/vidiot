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

#ifndef HELPER_TIMELINE_ASSERT_H
#define HELPER_TIMELINE_ASSERT_H

#include "HelperTimeline.h"

namespace test {

/// Assert that there are no more transitions in the given track
void ASSERT_NO_TRANSITIONS_IN_VIDEO_TRACK(int trackindex = 0);

/// Assert for the count of the selected clips.
/// Named such for readibility of test cases.
/// Note that the current selected clips (getSelectedClipsCount())
/// are compared to size * 2 (to avoid having to duplicate for
/// the combination of audio and video clips throughout the tests).
void ASSERT_SELECTION_SIZE(int size);

//////////////////////////////////////////////////////////////////////////
// CLIPTYPEASSERTER
//////////////////////////////////////////////////////////////////////////

struct ClipTypeAsserter
    :   boost::noncopyable
{
    ClipTypeAsserter& CLIPTYPEASSERTER_A;   ///< Helper, in order to be able to compile the code (TYPEASSERTER_* macros)
    ClipTypeAsserter& CLIPTYPEASSERTER_B;   ///< Helper, in order to be able to compile the code (TYPEASSERTER_* macros)

    ClipTypeAsserter(bool video, int TrackNumber, int ClipNumber)
        :   CLIPTYPEASSERTER_A(*this)
        ,   CLIPTYPEASSERTER_B(*this)
        ,   mVideo(video)
        ,   mTrackNumber(TrackNumber)
        ,   mClipNumber(ClipNumber)
    {
    }

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

    void ClipTypeMismatch(const type_info& expectedtype);

    int mTrackNumber;
    int mClipNumber;
    bool mVideo;
};

#define CLIPTYPEASSERTER_A(type) CLIPTYPEASSERTER_OP(type, B)
#define CLIPTYPEASSERTER_B(type) CLIPTYPEASSERTER_OP(type, A)
#define CLIPTYPEASSERTER_OP(type, next) CLIPTYPEASSERTER_A.AssertClipType<model::type>().CLIPTYPEASSERTER_ ## next
#define ASSERT_VIDEOTRACK(TrackNumber,ClipNumber) ClipTypeAsserter(true, TrackNumber,ClipNumber).CLIPTYPEASSERTER_A
#define ASSERT_AUDIOTRACK(TrackNumber,ClipNumber) ClipTypeAsserter(false,TrackNumber,ClipNumber).CLIPTYPEASSERTER_A
#define ASSERT_VIDEOTRACK1 ASSERT_VIDEOTRACK(1,0)
#define ASSERT_VIDEOTRACK0 ASSERT_VIDEOTRACK(0,0)
#define ASSERT_AUDIOTRACK0 ASSERT_AUDIOTRACK(0,0)
#define ASSERT_AUDIOTRACK1 ASSERT_AUDIOTRACK(1,0)

} // namespace

#endif // HELPER_TIMELINE_ASSERT_H