#include "HelperTransition.h"

#include "Config.h"
#include "CreateTransition.h"
#include "HelperTimeline.h"
#include "HelperTimelinesView.h"
#include "HelperWindow.h"
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

MakeTransitionAfterClip::MakeTransitionAfterClip(int afterclip)
    : clipNumberBeforeTransition(afterclip)
{
    storeVariablesBeforeTrimming();
}

MakeTransitionAfterClip::~MakeTransitionAfterClip()
{
}

void MakeTransitionAfterClip::makeTransition()
{
    storeVariablesBeforeMakingTransition();
    PositionCursor(LeftPixel(VideoClip(0,clipNumberBeforeTransition + 1)));
    Move(LeftCenter(VideoClip(0,clipNumberBeforeTransition + 1)));
    Type('c');
    storeVariablesAfterMakingTransition();
}

void MakeTransitionAfterClip::storeVariablesBeforeTrimming()
{
    leftPositionOfClipBeforeTransitionOriginal = LeftPixel(VideoClip(0,clipNumberBeforeTransition));
    leftPositionOfClipAfterTransitionOriginal  = LeftPixel(VideoClip(0,clipNumberBeforeTransition + 1));
    lengthOfClipBeforeTransitionOriginal = VideoClip(0,clipNumberBeforeTransition)->getLength();
    lengthOfClipAfterTransitionOriginal  = VideoClip(0,clipNumberBeforeTransition + 1)->getLength();
}

void MakeTransitionAfterClip::storeVariablesBeforeMakingTransition()
{
    leftPositionOfClipBeforeTransitionBeforeApplyingTransition  = LeftPixel(VideoClip(0,clipNumberBeforeTransition));
    leftPositionOfClipAfterTransitionBeforeApplyingTransition   = LeftPixel(VideoClip(0,clipNumberBeforeTransition + 1));
    lengthOfFirstClip                                           = VideoClip(0,0)->getLength(); // This is a fixed index, since really the first clip's length is used
    lengthOfClipBeforeTransitionBeforeTransitionApplied         = VideoClip(0,clipNumberBeforeTransition)->getLength();
    lengthOfClipAfterTransitionBeforeTransitionApplied          = VideoClip(0,clipNumberBeforeTransition + 1)->getLength();
}

void MakeTransitionAfterClip::storeVariablesAfterMakingTransition()
{
    leftPositionOfClipBeforeTransitionAfterTransitionApplied = LeftPixel(VideoClip(0,clipNumberBeforeTransition));
    leftPositionOfTransitionAfterTransitionApplied           = LeftPixel(VideoClip(0,clipNumberBeforeTransition + 1));
    leftPositionOfClipAfterTransitionAfterTransitionApplied  = LeftPixel(VideoClip(0,clipNumberBeforeTransition + 2));
    lengthOfClipBeforeTransitionAfterTransitionApplied       = VideoClip(0,clipNumberBeforeTransition)->getLength();
    lengthOfClipAfterTransitionAfterTransitionApplied        = VideoClip(0,clipNumberBeforeTransition + 2)->getLength();
    model::TransitionPtr transition                          = boost::dynamic_pointer_cast<model::Transition>(VideoClip(0,clipNumberBeforeTransition + 1));
    touchPositionOfTransition                                = getTimeline().getZoom().ptsToPixels(transition->getTouchPosition());
    lengthOfTransition                                       = transition->getLength();
}

MakeInOutTransitionAfterClip::MakeInOutTransitionAfterClip(int afterclip)
    : MakeTransitionAfterClip(afterclip)
{
    // Reduce size of clips to be able to create transition
    TrimLeft(VideoClip(0,clipNumberBeforeTransition + 1),30,true);
    TrimRight(VideoClip(0,clipNumberBeforeTransition),30,true);
    ASSERT_MORE_THAN_ZERO(VideoClip(0,clipNumberBeforeTransition)->getMaxAdjustEnd())(VideoClip(0,clipNumberBeforeTransition));
    ASSERT_LESS_THAN_ZERO(VideoClip(0,clipNumberBeforeTransition + 1)->getMinAdjustBegin())(VideoClip(0,clipNumberBeforeTransition + 1));

    makeTransition();

    ASSERT(VideoClip(0,clipNumberBeforeTransition + 1)->isA<model::Transition>())(VideoClip(0,clipNumberBeforeTransition + 1));
    ASSERT_EQUALS(lengthOfTransition, gui::Config::ReadLong(gui::Config::sPathDefaultTransitionLength));
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

MakeInTransitionAfterClip::MakeInTransitionAfterClip(int afterclip)
    : MakeTransitionAfterClip(afterclip)
{
    // Reduce size of clips to be able to create transition
    TrimRight(VideoClip(0,clipNumberBeforeTransition),30,true);
    ASSERT_MORE_THAN_ZERO(VideoClip(0,clipNumberBeforeTransition)->getMaxAdjustEnd())(VideoClip(0,clipNumberBeforeTransition));
    ASSERT_ZERO(VideoClip(0,clipNumberBeforeTransition + 1)->getMinAdjustBegin())(VideoClip(0,clipNumberBeforeTransition + 1));

    makeTransition();

    ASSERT(VideoClip(0,clipNumberBeforeTransition + 1)->isA<model::Transition>())(VideoClip(0,clipNumberBeforeTransition + 1));
    ASSERT_EQUALS(lengthOfTransition, gui::Config::ReadLong(gui::Config::sPathDefaultTransitionLength) / 2);
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

MakeOutTransitionAfterClip::MakeOutTransitionAfterClip(int afterclip)
    : MakeTransitionAfterClip(afterclip)
{
    // Reduce size of clips to be able to create transition
    TrimLeft(VideoClip(0,clipNumberBeforeTransition + 1),30,true);
    ASSERT_ZERO(VideoClip(0,clipNumberBeforeTransition)->getMaxAdjustEnd())(VideoClip(0,clipNumberBeforeTransition));
    ASSERT_LESS_THAN_ZERO(VideoClip(0,clipNumberBeforeTransition + 1)->getMinAdjustBegin())(VideoClip(0,clipNumberBeforeTransition + 1));

    makeTransition();

    ASSERT(VideoClip(0,clipNumberBeforeTransition + 1)->isA<model::Transition>())(VideoClip(0,clipNumberBeforeTransition + 1));
    ASSERT_EQUALS(lengthOfTransition, gui::Config::ReadLong(gui::Config::sPathDefaultTransitionLength) / 2);
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