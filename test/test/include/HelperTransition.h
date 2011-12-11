#ifndef HELPER_TRANSITION_H
#define HELPER_TRANSITION_H

#include <wx/gdicmn.h>
#include <boost/shared_ptr.hpp>
#include "UtilInt.h"

namespace model {
class IClip;
typedef boost::shared_ptr<IClip> IClipPtr;
}

namespace test {

wxPoint TransitionLeftClipInterior(model::IClipPtr clip);
wxPoint TransitionLeftClipEnd(model::IClipPtr clip);
wxPoint TransitionRightClipInterior(model::IClipPtr clip);
wxPoint TransitionRightClipBegin(model::IClipPtr clip);

/// Base class for reusing several variables
/// and the creation of the trimmed clips before creating
/// the transition.
struct MakeTransitionAfterClip
{
    explicit MakeTransitionAfterClip(int afterclip);
    virtual ~MakeTransitionAfterClip();

    pts clipNumberBeforeTransition;

    pts leftPositionOfClipBeforeTransitionBeforeApplyingTransition;
    pts leftPositionOfClipAfterTransitionBeforeApplyingTransition;
    pts defaultSize;
    pts lengthOfFirstClip;

    pts lengthOfClipBeforeTransitionBeforeTransitionApplied;
    pts lengthOfClipAfterTransitionBeforeTransitionApplied;

    pixel leftPositionOfClipBeforeTransitionAfterTransitionApplied;
    pixel leftPositionOfTransitionAfterTransitionApplied;
    pixel leftPositionOfClipAfterTransitionAfterTransitionApplied;

    pts lengthOfClipBeforeTransitionAfterTransitionApplied;
    pts lengthOfClipAfterTransitionAfterTransitionApplied;

    pixel touchPositionOfTransition;

protected:

    /// Makes a transition by moving the mouse to the position where it
    /// must be made and then pressing 'c'.
    void makeTransition();

private:

    void storeVariablesBeforeMakingTransition();
    void storeVariablesAfterMakingTransition();
};

struct MakeInOutTransitionAfterClip : public MakeTransitionAfterClip
{
    MakeInOutTransitionAfterClip(int afterclip);
    ~MakeInOutTransitionAfterClip();
};

struct MakeInTransitionAfterClip : public MakeTransitionAfterClip // tod rename to make in before clip...
{
    MakeInTransitionAfterClip(int afterclip);
    ~MakeInTransitionAfterClip();
};

struct MakeOutTransitionAfterClip : public MakeTransitionAfterClip
{
    MakeOutTransitionAfterClip(int afterclip);
    ~MakeOutTransitionAfterClip();
};

} // namespace

#endif // HELPER_TIMELINE_H