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

#include "Test.h"

#pragma warning(disable:4996)

namespace test {

void AssertNoTransitionsInVideoTrack(int trackindex)
{
    for (int i = 0; i < NumberOfVideoClipsInTrack(trackindex); ++i)
    {
        ASSERT(!VideoClip(trackindex,i)->isA<model::Transition>())(i);
    }
}

void AssertSelectionSize(int size)
{
    ASSERT_EQUALS(getSelectedClipsCount(),2 * size); // * 2 since AudioClips are selected also
}

void AssertSelection(model::IClips expected)
{
    std::set<model::IClipPtr> current{ getSequence()->getSelectedClips() };
    model::IClips actual; 
    actual.assign(current.begin(), current.end());
    std::sort(actual.begin(), actual.end());
    std::sort(expected.begin(), expected.end());
    ASSERT_EQUALS(actual, expected);
}

//////////////////////////////////////////////////////////////////////////
// CLIPTYPEASSERTER
//////////////////////////////////////////////////////////////////////////

ClipTypeAsserter::~ClipTypeAsserter()
{
};

void ClipTypeAsserter::ClipTypeMismatch(const std::type_info& expectedtype)
{
    int TrackNumber = mTrackNumber;
    int ClipNumber = mClipNumber;
    std::string TrackType = mVideo ? "VIDEO" : "AUDIO";
    auto convert = [](const std::type_info& info) -> std::string
    {
        std::string theName = info.name();
        return theName.substr(theName.find_last_of(':') + 1);
    };

    std::string Expected = convert(expectedtype);
    std::string Got = mVideo ? convert(typeid(*VideoClip(TrackNumber,mClipNumber))) : convert(typeid(*AudioClip(TrackNumber, mClipNumber)));;
    LogVar("Clip type error", __FILE__, __LINE__,__FUNCTION__).LOGVAR_A(TrackType)(TrackNumber)(ClipNumber)(Expected)(Got);
}

} // namespace
