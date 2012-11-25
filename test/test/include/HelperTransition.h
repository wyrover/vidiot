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
    explicit MakeTransitionAfterClip(int afterclip, bool audio);
    virtual ~MakeTransitionAfterClip();

    pts clipNumberBeforeTransition;

    // Before trimming the clips to make room for the transition
    pts leftPositionOfClipBeforeTransitionOriginal;
    pts leftPositionOfClipAfterTransitionOriginal;
    pts lengthOfClipBeforeTransitionOriginal;
    pts lengthOfClipAfterTransitionOriginal;

    // Before making the transition
    pts leftPositionOfClipBeforeTransitionBeforeApplyingTransition;
    pts leftPositionOfClipAfterTransitionBeforeApplyingTransition;
    pts lengthOfFirstClip;
    pts lengthOfClipBeforeTransitionBeforeTransitionApplied;
    pts lengthOfClipAfterTransitionBeforeTransitionApplied;

    // After making the transition
    pixel leftPositionOfClipBeforeTransitionAfterTransitionApplied;
    pixel leftPositionOfTransitionAfterTransitionApplied;
    pixel leftPositionOfClipAfterTransitionAfterTransitionApplied;
    pts lengthOfClipBeforeTransitionAfterTransitionApplied;
    pts lengthOfClipAfterTransitionAfterTransitionApplied;
    pts lengthOfTransition;
    pixel touchPositionOfTransition;

protected:

    /// Makes a transition by moving the mouse to the position where it
    /// must be made and then pressing 'c'.
    void makeTransition();

    model::IClipPtr GetClip(int track, int clip) const;

private:

    void storeVariablesBeforeTrimming();
    void storeVariablesBeforeMakingTransition();
    void storeVariablesAfterMakingTransition();

    bool mAudio;

};

struct MakeInOutTransitionAfterClip : public MakeTransitionAfterClip
{
    MakeInOutTransitionAfterClip(int afterclip, bool audio = false);
    ~MakeInOutTransitionAfterClip();
};

struct MakeInTransitionAfterClip : public MakeTransitionAfterClip // tod rename to make in before clip...
{
    MakeInTransitionAfterClip(int afterclip, bool audio = false);
    ~MakeInTransitionAfterClip();
};

struct MakeOutTransitionAfterClip : public MakeTransitionAfterClip
{
    MakeOutTransitionAfterClip(int afterclip, bool audio = false);
    ~MakeOutTransitionAfterClip();
};

} // namespace

#endif // HELPER_TIMELINE_H