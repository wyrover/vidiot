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

void MakeVideoTransitionLeftPart0(int trackNumber, int clipNumber)
{
    model::TransitionPtr transitionBefore = VideoTransition(trackNumber,clipNumber);
    ASSERT(transitionBefore);
    ASSERT_MORE_THAN_ZERO(*(transitionBefore->getLeft()));
    ASSERT_MORE_THAN_ZERO(*(transitionBefore->getRight()));
    wxPoint from = VTopQuarterLeft(VideoClip(trackNumber,clipNumber));
    TimelineTrim(from,from + wxPoint(100,0));
    model::TransitionPtr transitionAfter = VideoTransition(trackNumber,clipNumber);
    ASSERT_ZERO(*(transitionAfter->getLeft()));
    ASSERT_MORE_THAN_ZERO(*(transitionAfter->getRight()));
}

void MakeVideoTransitionRightPart0(int trackNumber, int clipNumber)
{
    model::TransitionPtr transitionBefore = VideoTransition(trackNumber,clipNumber);
    ASSERT(transitionBefore);
    ASSERT_MORE_THAN_ZERO(*(transitionBefore->getLeft()));
    ASSERT_MORE_THAN_ZERO(*(transitionBefore->getRight()));
    wxPoint from = VTopQuarterRight(VideoClip(trackNumber,clipNumber));
    TimelineTrim(from,from + wxPoint(-100,0));
    model::TransitionPtr transitionAfter = VideoTransition(trackNumber,clipNumber);
    ASSERT_ZERO(*(transitionAfter->getRight()));
    ASSERT_MORE_THAN_ZERO(*(transitionAfter->getLeft()));
}

void ASSERT_LOGICALCLIPPOSITION(wxPoint position, gui::timeline::MouseOnClipPosition expected)
{
    // This assert is done to detect some 'clicking' problems as soon as possible. With some zoom
    // values, using Transition* methods actually return a previous or next clip (typically happens
    // when not zoomed in enough).
    gui::timeline::PointerPositionInfo info = getTimeline().getMouse().getInfo(position);
    ASSERT_EQUALS(expected, info.logicalclipposition);
}

wxPoint TransitionLeftClipInterior(model::IClipPtr clip)
{
    ASSERT(clip->isA<model::Transition>());
    wxPoint position = LeftBottom(clip);
    ASSERT_LOGICALCLIPPOSITION(position,gui::timeline::TransitionLeftClipInterior);
    return position;
}

wxPoint TransitionLeftClipEnd(model::IClipPtr clip)
{
    ASSERT(clip->isA<model::Transition>());
    model::TransitionPtr transition = boost::dynamic_pointer_cast<model::Transition>(clip);
    wxPoint position = wxPoint(getTimeline().getZoom().ptsToPixels(transition->getTouchPosition()) - 1, VBottomQuarter(clip->getTrack()));
    ASSERT_LOGICALCLIPPOSITION(position, gui::timeline::TransitionLeftClipEnd);
    return position;
}

wxPoint TransitionRightClipInterior(model::IClipPtr clip)
{
    ASSERT(clip->isA<model::Transition>());
    wxPoint position = RightBottom(clip);
    ASSERT_LOGICALCLIPPOSITION(position, gui::timeline::TransitionRightClipInterior);
    return position;
}

wxPoint TransitionRightClipBegin(model::IClipPtr clip)
{
    ASSERT(clip->isA<model::Transition>());
    model::TransitionPtr transition = boost::dynamic_pointer_cast<model::Transition>(clip);
    wxPoint position = wxPoint(getTimeline().getZoom().ptsToPixels(transition->getTouchPosition()), VBottomQuarter(clip->getTrack()));
    ASSERT_LOGICALCLIPPOSITION(position, gui::timeline::TransitionRightClipBegin);
    return position;
}

pixel TransitionCut(model::IClipPtr clip)
{
    return TransitionRightClipBegin(clip).x;
}

pts DefaultTransitionLength()
{
    static pts result{ util::thread::RunInMainReturning<pts>([]
    {
        return Config::ReadLong(Config::sPathTimelineDefaultTransitionLength);
    }) };
    return result;
}

MakeTransitionAfterClip::MakeTransitionAfterClip(int afterclip, bool audio)
    : clipNumberBeforeTransition(afterclip)
    , clipNumberAfterTransition(afterclip+1)
    , mAudio(audio)
    , mUndo(true)
{
    storeVariablesBeforeTrimming();
}

MakeTransitionAfterClip::~MakeTransitionAfterClip()
{
}

void MakeTransitionAfterClip::dontUndo()
{
    mUndo = false;
}

void MakeTransitionAfterClip::makeTransition()
{
    ASSERT_SELECTION_SIZE(0); // Done to avoid 'leaving' selected clips which cause other tests to fail
    storeVariablesBeforeMakingTransition();
    TimelinePositionCursor(LeftPixel(GetClip(0,clipNumberAfterTransition)));
    moveMouseAndCreateTransition(clipNumberAfterTransition);
    storeVariablesAfterMakingTransition();
}

model::IClipPtr MakeTransitionAfterClip::GetClip(int track, int clip) const
{
    model::IClipPtr result;
    util::thread::RunInMainAndWait([&result, this, track, clip]
    {
        result = mAudio ? AudioClip(track,clip) : VideoClip(track,clip);
    });
    ASSERT(result != nullptr)(track)(clip);
    return result;
}

void MakeTransitionAfterClip::storeVariablesBeforeTrimming()
{
    leftPositionOfClipBeforeTransitionOriginal = LeftPixel(GetClip(0,clipNumberBeforeTransition));
    leftPositionOfClipAfterTransitionOriginal  = LeftPixel(GetClip(0,clipNumberAfterTransition));
    lengthOfClipBeforeTransitionOriginal = GetClip(0,clipNumberBeforeTransition)->getLength();
    lengthOfClipAfterTransitionOriginal  = GetClip(0,clipNumberAfterTransition)->getLength();
}

void MakeTransitionAfterClip::storeVariablesBeforeMakingTransition()
{
    leftPositionOfClipBeforeTransitionBeforeApplyingTransition  = LeftPixel(GetClip(0,clipNumberBeforeTransition));
    leftPositionOfClipAfterTransitionBeforeApplyingTransition   = LeftPixel(GetClip(0,clipNumberAfterTransition));
    lengthOfFirstClip                                           = GetClip(0,0)->getLength(); // This is a fixed index, since really the first clip's length is used
    lengthOfClipBeforeTransitionBeforeTransitionApplied         = GetClip(0,clipNumberBeforeTransition)->getLength();
    lengthOfClipAfterTransitionBeforeTransitionApplied          = GetClip(0,clipNumberAfterTransition)->getLength();
}

void MakeTransitionAfterClip::storeVariablesAfterMakingTransition()
{
    leftPositionOfClipBeforeTransitionAfterTransitionApplied = LeftPixel(GetClip(0,clipNumberBeforeTransition));
    leftPositionOfTransitionAfterTransitionApplied           = LeftPixel(GetClip(0,clipNumberAfterTransition));
    leftPositionOfClipAfterTransitionAfterTransitionApplied  = LeftPixel(GetClip(0,clipNumberAfterTransition + 1));
    lengthOfClipBeforeTransitionAfterTransitionApplied       = GetClip(0,clipNumberBeforeTransition)->getLength();
    lengthOfClipAfterTransitionAfterTransitionApplied        = GetClip(0,clipNumberAfterTransition + 1)->getLength();
    model::TransitionPtr transition                          = boost::dynamic_pointer_cast<model::Transition>(GetClip(0,clipNumberAfterTransition));
    ASSERT(transition);
    touchPositionOfTransition                                = getTimeline().getZoom().ptsToPixels(transition->getTouchPosition());
    lengthOfTransition                                       = transition->getLength();
}

MakeInOutTransitionAfterClip::MakeInOutTransitionAfterClip(int afterclip, bool audio)
    : MakeTransitionAfterClip(afterclip, audio)
{
    ConfigOverrule<bool> overruleSnapToCursor(Config::sPathTimelineSnapClips,false);
    ConfigOverrule<bool> overruleSnapToClips(Config::sPathTimelineSnapCursor,false);

    // Reduce size of clips to be able to create transition
    TimelineTrimLeft(GetClip(0,clipNumberAfterTransition),30,true);
    ASSERT_LESS_THAN_ZERO(GetClip(0,clipNumberAfterTransition)->getMinAdjustBegin())(GetClip(0,clipNumberAfterTransition));
    TimelineTrimRight(GetClip(0,clipNumberBeforeTransition),-30,true);
    ASSERT_MORE_THAN_ZERO(GetClip(0,clipNumberBeforeTransition)->getMaxAdjustEnd())(GetClip(0,clipNumberBeforeTransition));
    TimelineDeselectAllClips();

    makeTransition();

    ASSERT(GetClip(0,clipNumberAfterTransition)->isA<model::Transition>())(GetClip(0,clipNumberAfterTransition));
    ASSERT_EQUALS(lengthOfTransition, DefaultTransitionLength());
    ASSERT_EQUALS(lengthOfClipBeforeTransitionAfterTransitionApplied, lengthOfClipBeforeTransitionBeforeTransitionApplied - lengthOfTransition / 2);
    ASSERT_EQUALS(lengthOfClipAfterTransitionAfterTransitionApplied, lengthOfClipAfterTransitionBeforeTransitionApplied - lengthOfTransition / 2);
}

MakeInOutTransitionAfterClip::~MakeInOutTransitionAfterClip()
{
    if (mUndo)
    {
        ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::CreateTransition>();
        Undo(); // Undo create transition
        ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::TrimClip>();
        Undo(); // Undo TimelineTrimRight
        ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::TrimClip>();
        Undo(); // Undo TimelineTrimLeft
        TimelineDeselectAllClips(); // Done to avoid 'leaving' selected clips which cause other tests to fail
    }
}

void MakeInOutTransitionAfterClip::moveMouseAndCreateTransition(int clipNumber)
{
    TimelineMove(LeftCenter(GetClip(0,clipNumber)));
    TimelineKeyPress('p');
}

MakeInTransitionAfterClip::MakeInTransitionAfterClip(int afterclip, bool audio)
    : MakeTransitionAfterClip(afterclip, audio)
{
    ConfigOverrule<bool> overruleSnapToCursor(Config::sPathTimelineSnapClips,false);
    ConfigOverrule<bool> overruleSnapToClips(Config::sPathTimelineSnapCursor,false);

    makeTransition();

    ASSERT(GetClip(0,clipNumberAfterTransition)->isA<model::Transition>())(GetClip(0,clipNumberAfterTransition));
    ASSERT_EQUALS(lengthOfTransition, DefaultTransitionLength() / 2);
    ASSERT_EQUALS(lengthOfClipBeforeTransitionAfterTransitionApplied, lengthOfClipBeforeTransitionBeforeTransitionApplied);
    ASSERT_EQUALS(lengthOfClipAfterTransitionAfterTransitionApplied, lengthOfClipAfterTransitionBeforeTransitionApplied - lengthOfTransition);
}

MakeInTransitionAfterClip::~MakeInTransitionAfterClip()
{
    if (mUndo)
    {
        ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::CreateTransition>();
        Undo(); // Undo create transition
        TimelineDeselectAllClips(); // Done to avoid 'leaving' selected clips which cause other tests to fail
    }
}

void MakeInTransitionAfterClip::moveMouseAndCreateTransition(int clipNumber)
{
    TimelineMove(LeftCenter(GetClip(0,clipNumberAfterTransition)));
    TimelineKeyPress('i');
}

MakeOutTransitionAfterClip::MakeOutTransitionAfterClip(int afterclip, bool audio)
    : MakeTransitionAfterClip(afterclip, audio)
{
    ConfigOverrule<bool> overruleSnapToCursor(Config::sPathTimelineSnapClips,false);
    ConfigOverrule<bool> overruleSnapToClips(Config::sPathTimelineSnapCursor,false);

    makeTransition();

    ASSERT(GetClip(0,clipNumberAfterTransition)->isA<model::Transition>())(GetClip(0,clipNumberAfterTransition));
    ASSERT_EQUALS(lengthOfTransition, DefaultTransitionLength() / 2);
    ASSERT_EQUALS(lengthOfClipBeforeTransitionAfterTransitionApplied, lengthOfClipBeforeTransitionBeforeTransitionApplied - lengthOfTransition);
    ASSERT_EQUALS(lengthOfClipAfterTransitionAfterTransitionApplied, lengthOfClipAfterTransitionBeforeTransitionApplied);
}

MakeOutTransitionAfterClip::~MakeOutTransitionAfterClip()
{
    if (mUndo)
    {
        ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::CreateTransition>();
        Undo(); // Undo create transition
        TimelineDeselectAllClips(); // Done to avoid 'leaving' selected clips which cause other tests to fail
    }
}

void MakeOutTransitionAfterClip::moveMouseAndCreateTransition(int clipNumber)
{
    TimelineMove(LeftCenter(GetClip(0,clipNumber - 1)));
    TimelineKeyPress('o');
}

} // namespace
