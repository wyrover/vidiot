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

#ifndef HELPER_TRANSITION_H
#define HELPER_TRANSITION_H

#include "Test.h"

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

    void dontUndo();

    int clipNumberBeforeTransition;
    int clipNumberAfterTransition;

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
    /// must be made and then pressing the corresponding key.
    void makeTransition();

    virtual void moveMouseAndCreateTransition(int clipNumber) = 0;

    model::IClipPtr GetClip(int track, int clip) const;

    bool mUndo;

private:

    void storeVariablesBeforeTrimming();
    void storeVariablesBeforeMakingTransition();
    void storeVariablesAfterMakingTransition();

    bool mAudio;

};

struct MakeInOutTransitionAfterClip : public MakeTransitionAfterClip
{
    MakeInOutTransitionAfterClip(int afterclip, bool audio = false);
    virtual ~MakeInOutTransitionAfterClip();
    virtual void moveMouseAndCreateTransition(int clipNumber) override;
};

struct MakeInTransitionAfterClip : public MakeTransitionAfterClip // tod rename to make in before clip...
{
    MakeInTransitionAfterClip(int afterclip, bool audio = false);
    virtual ~MakeInTransitionAfterClip();
    virtual void moveMouseAndCreateTransition(int clipNumber) override;
};

struct MakeOutTransitionAfterClip : public MakeTransitionAfterClip
{
    MakeOutTransitionAfterClip(int afterclip, bool audio = false);
    virtual ~MakeOutTransitionAfterClip();
    virtual void moveMouseAndCreateTransition(int clipNumber) override;
};

} // namespace

#endif