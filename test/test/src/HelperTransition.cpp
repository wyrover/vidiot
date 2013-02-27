#include "HelperTransition.h"

#include "Config.h"
#include "CreateTransition.h"
#include "HelperConfig.h"
#include "HelperTimeline.h"
#include "HelperTimelinesView.h"
#include "HelperWindow.h"
#include "ids.h"
#include "Layout.h"
#include "MousePointer.h"
#include "PositionInfo.h"
#include "PositionInfo.h"
#include "Sequence.h"
#include "Timeline.h"
#include "Transition.h"
#include "TrimClip.h"
#include "VideoClip.h"
#include "Zoom.h"

namespace test {

void ASSERT_LOGICALCLIPPOSITION(wxPoint position, gui::timeline::MouseOnClipPosition expected)
{
    // This assert is done to detect some 'clicking' problems as soon as possible. With some zoom
    // values, using Transition* methods actually return a previous or next clip (typically happens
    // when not zoomed in enough).
    gui::timeline::PointerPositionInfo info = getTimeline().getMousePointer().getInfo(position);
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
    wxPoint position = wxPoint(getTimeline().getZoom().ptsToPixels(transition->getTouchPosition()) - 1,BottomPixel(clip));
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
    wxPoint position = wxPoint(getTimeline().getZoom().ptsToPixels(transition->getTouchPosition()),BottomPixel(clip));
    ASSERT_LOGICALCLIPPOSITION(position, gui::timeline::TransitionRightClipBegin);
    return position;
}

MakeTransitionAfterClip::MakeTransitionAfterClip(int afterclip, bool audio)
    : clipNumberBeforeTransition(afterclip)
    , clipNumberAfterTransition(afterclip+1)
    , mAudio(audio)
{
    storeVariablesBeforeTrimming();
}

MakeTransitionAfterClip::~MakeTransitionAfterClip()
{
}

void MakeTransitionAfterClip::makeTransition()
{
    storeVariablesBeforeMakingTransition();
    PositionCursor(LeftPixel(GetClip(0,clipNumberAfterTransition)));
    Move(LeftCenter(GetClip(0,clipNumberAfterTransition)));
    Type('c');
    storeVariablesAfterMakingTransition();
}

model::IClipPtr MakeTransitionAfterClip::GetClip(int track, int clip) const
{
    if (mAudio)
    {
        return AudioClip(track,clip);
    }
    return VideoClip(track,clip);

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
    ConfigOverruleBool overruleSnapToCursor(Config::sPathSnapClips,false);
    ConfigOverruleBool overruleSnapToClips(Config::sPathSnapCursor,false);

    // Reduce size of clips to be able to create transition
    TrimLeft(GetClip(0,clipNumberAfterTransition),30,true);
    TrimRight(GetClip(0,clipNumberBeforeTransition),-30,true);
    ASSERT_MORE_THAN_ZERO(GetClip(0,clipNumberBeforeTransition)->getMaxAdjustEnd())(GetClip(0,clipNumberBeforeTransition));
    ASSERT_LESS_THAN_ZERO(GetClip(0,clipNumberAfterTransition)->getMinAdjustBegin())(GetClip(0,clipNumberAfterTransition));

    makeTransition();

    ASSERT(GetClip(0,clipNumberAfterTransition)->isA<model::Transition>())(GetClip(0,clipNumberAfterTransition));
    ASSERT_EQUALS(lengthOfTransition, Config::ReadLong(Config::sPathDefaultTransitionLength));
    ASSERT_EQUALS(lengthOfClipBeforeTransitionAfterTransitionApplied, lengthOfClipBeforeTransitionBeforeTransitionApplied - lengthOfTransition / 2);
    ASSERT_EQUALS(lengthOfClipAfterTransitionAfterTransitionApplied, lengthOfClipAfterTransitionBeforeTransitionApplied - lengthOfTransition / 2);
}

MakeInOutTransitionAfterClip::~MakeInOutTransitionAfterClip()
{
    ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::CreateTransition>();
    Undo(); // Undo create transition
    ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::TrimClip>();
    Undo(); // Undo TrimRight
    ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::TrimClip>();
    Undo(); // Undo TrimLeft
}

MakeInTransitionAfterClip::MakeInTransitionAfterClip(int afterclip, bool audio)
    : MakeTransitionAfterClip(afterclip, audio)
{
    ConfigOverruleBool overruleSnapToCursor(Config::sPathSnapClips,false);
    ConfigOverruleBool overruleSnapToClips(Config::sPathSnapCursor,false);

    // Reduce size of clips to be able to create transition
    TrimRight(GetClip(0,clipNumberBeforeTransition),-30,true);
    ASSERT_MORE_THAN_ZERO(GetClip(0,clipNumberBeforeTransition)->getMaxAdjustEnd())(GetClip(0,clipNumberBeforeTransition));
    ASSERT_ZERO(GetClip(0,clipNumberAfterTransition)->getMinAdjustBegin())(GetClip(0,clipNumberAfterTransition));

    makeTransition();

    ASSERT(GetClip(0,clipNumberAfterTransition)->isA<model::Transition>())(GetClip(0,clipNumberAfterTransition));
    ASSERT_EQUALS(lengthOfTransition, Config::ReadLong(Config::sPathDefaultTransitionLength) / 2);
    ASSERT_EQUALS(lengthOfClipBeforeTransitionAfterTransitionApplied, lengthOfClipBeforeTransitionBeforeTransitionApplied);
    ASSERT_EQUALS(lengthOfClipAfterTransitionAfterTransitionApplied, lengthOfClipAfterTransitionBeforeTransitionApplied - lengthOfTransition);
}

MakeInTransitionAfterClip::~MakeInTransitionAfterClip()
{
    ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::CreateTransition>();
    Undo(); // Undo create transition
    ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::TrimClip>();
    Undo(); // Undo TrimRight
}

MakeOutTransitionAfterClip::MakeOutTransitionAfterClip(int afterclip, bool audio)
    : MakeTransitionAfterClip(afterclip, audio)
{
    ConfigOverruleBool overruleSnapToCursor(Config::sPathSnapClips,false);
    ConfigOverruleBool overruleSnapToClips(Config::sPathSnapCursor,false);

    // Reduce size of clips to be able to create transition
    TrimLeft(GetClip(0,clipNumberAfterTransition),30,true);
    ASSERT_ZERO(GetClip(0,clipNumberBeforeTransition)->getMaxAdjustEnd())(GetClip(0,clipNumberBeforeTransition));
    ASSERT_LESS_THAN_ZERO(GetClip(0,clipNumberAfterTransition)->getMinAdjustBegin())(GetClip(0,clipNumberAfterTransition));

    makeTransition();

    ASSERT(GetClip(0,clipNumberAfterTransition)->isA<model::Transition>())(GetClip(0,clipNumberAfterTransition));
    ASSERT_EQUALS(lengthOfTransition, Config::ReadLong(Config::sPathDefaultTransitionLength) / 2);
    ASSERT_EQUALS(lengthOfClipBeforeTransitionAfterTransitionApplied, lengthOfClipBeforeTransitionBeforeTransitionApplied - lengthOfTransition);
    ASSERT_EQUALS(lengthOfClipAfterTransitionAfterTransitionApplied, lengthOfClipAfterTransitionBeforeTransitionApplied);
}

MakeOutTransitionAfterClip::~MakeOutTransitionAfterClip()
{
    ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::CreateTransition>();
    Undo(); // Undo create transition
    ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::TrimClip>();
    Undo(); // Undo TrimLeft
}

} // namespace