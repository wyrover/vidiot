// Copyright 2015 Eric Raijmakers.
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

#include "EditClipSpeed.h"

#include "ClipInterval.h"
#include "Cursor.h"
#include "EmptyClip.h"
#include "IClip.h"
#include "Keyboard.h"
#include "Selection.h"
#include "Sequence.h"
#include "StatusBar.h"
#include "Timeline.h"
#include "Track.h"
#include "Transition.h"
#include "Trim.h"
#include "UtilClone.h"
#include "UtilLog.h"
#include "VideoClip.h"
#include "Zoom.h"

namespace gui { namespace timeline { namespace command {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

EditClipSpeed::EditClipSpeed(
    const model::SequencePtr& sequence,
    const model::IClipPtr& clip,
    const model::IClipPtr& link,
    const model::IClipPtr& clipClone,
    const model::IClipPtr& linkClone,
    rational speed)
    : AClipEdit(sequence)
    , mClip(clip)
    , mClipClone(clipClone)
    , mLink(link)
    , mLinkClone(linkClone)
    , mSpeed(speed)
    , mPossible(false)
{
    // todo make boost rational with 0 denominator. Assert is caught by exception filter, with strange stack trace. caused by #define wxUSE_EXCEPTIONS    1 where exception rethrown (inwxevtloopmanual:dorun) causes new call stack?
    VAR_INFO(this)(mClip)(mLink)(speed);
    ASSERT_MORE_THAN_ZERO(speed);
    ASSERT_DIFFERS(mClip,mClipClone);
    ASSERT(!mClip->isA<model::EmptyClip>());
    ASSERT(!mClip->isA<model::AudioClip>());
    ASSERT(!mClip->isA<model::Transition>());

    boost::shared_ptr<model::ClipInterval> clipInterval{ boost::dynamic_pointer_cast<model::ClipInterval>(mClipClone) };
    boost::shared_ptr<model::ClipInterval> linkInterval{ mLinkClone ? boost::dynamic_pointer_cast<model::ClipInterval>(mLinkClone) : nullptr };

    ASSERT(linkInterval == nullptr); // Only video speed can be changed for now.

    if (linkInterval)
    {
        if ((linkInterval->getSpeed() != clipInterval->getSpeed()) ||
            (linkInterval->getOffset() != clipInterval->getOffset()) ||
            (linkInterval->getLength() != clipInterval->getLength()))
        {
            gui::StatusBar::get().timedInfoText(_("Can not change length if start position, length, or speed for two linked clips are not equal."));
            return;
        }
    }

    boost::rational<int> originalSpeed = clipInterval->getSpeed();
    pts originalOffset = clipInterval->getOffset();
    pts originalLength = clipInterval->getLength();
    pts originalLeftPts = mClip->getLeftPts();
    pts originalRightPts = mClip->getRightPts();

    adjustSpeedForClipBounds(clip);
    adjustSpeedForClipBounds(link);

    if (mSpeed == clipInterval->getSpeed()) 
    { 
        return; 
    }

    // Adjust speed
    clipInterval->setSpeed(mSpeed);
    if (linkInterval != nullptr) { linkInterval->setSpeed(mSpeed); }

    // Update tracks holding clip and link
    replaceClip(mClip, { mClipClone });
    if (mLink != nullptr) { replaceClip(mLink, { mLinkClone }); }

    // Update other tracks
    pts diff{ mClip->getLength() - mClipClone->getLength() };
    for (model::TrackPtr track : getSequence()->getTracks())
    {
        if (track == mClipClone->getTrack()) { continue; }
        if (mLinkClone && track == mLinkClone->getTrack()) { continue; }

        // Get the clip that is at the clip's begin position
        model::IClipPtr clipInOtherTrackAtBegin = track->getClip(originalLeftPts);

        if (!clipInOtherTrackAtBegin) { continue; } // If no clip there (end of track). OK.                                      

        if (!clipInOtherTrackAtBegin->isA<model::EmptyClip>() ||         // If it's not an empty clip, then not OK.
            clipInOtherTrackAtBegin->getRightPts() < originalRightPts)   // If it's an empty clip, but not large enough, then not OK.  
        {
            gui::StatusBar::get().timedInfoText(_("Can not change clip speed. There may not be clips in the same timeframe in other tracks."));
            return;
        }

        replaceClip(clipInOtherTrackAtBegin, { boost::make_shared<model::EmptyClip>(clipInOtherTrackAtBegin->getLength() - diff) });
    }

    mCommandName = _("Adjust speed of ")  + mClip->getDescription();
    mPossible = true;
}

EditClipSpeed::~EditClipSpeed()
{
    if (!isInitialized())
    {
        Revert();
    }
}

//////////////////////////////////////////////////////////////////////////
// ROOTCOMMAND 
//////////////////////////////////////////////////////////////////////////

bool EditClipSpeed::isPossible() 
{
    return mPossible;
}

//////////////////////////////////////////////////////////////////////////
// ACLIPEDIT INTERFACE
//////////////////////////////////////////////////////////////////////////

void EditClipSpeed::initialize()
{
    // Already done in 'update'. This is called afterwards when the command is submitted.
}

void EditClipSpeed::doExtraAfter()
{
    // todo cursor pos changed to show exact same frame?

    //if (mCursorPositionBefore != mCursorPositionAfter)
    //{
    //    getTimeline().getCursor().setLogicalPosition(mCursorPositionAfter);
    //}
}

void EditClipSpeed::undoExtraAfter()
{
    //if (mCursorPositionBefore != mCursorPositionAfter)
    //{
    //    getTimeline().getCursor().setLogicalPosition(mCursorPositionBefore);
    //}
}
// todo add minidumps for debugging

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

model::IClipPtr EditClipSpeed::getClip() const
{
    return mClip;
}

rational EditClipSpeed::getActualSpeed() const
{
    return mSpeed;
}


wxString toString(rational speed)
{
    std::ostringstream os; os << std::setprecision(2) << std::fixed << boost::rational_cast<double>(speed);
    return wxString(os.str());
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void EditClipSpeed::adjustSpeedForClipBounds(model::IClipPtr clip)
{
    if (!clip) { return; } // Any speed change allowed
    ASSERT_NONZERO(clip->getTrack())(clip); // Required for checking adjacent transitions

    model::ClipIntervalPtr clipInterval{ boost::dynamic_pointer_cast<model::ClipInterval>(clip) };
    ASSERT_NONZERO(clipInterval);

    if (mSpeed < clipInterval->getSpeed()) { return; } // Enlarging clip via reducing size is always possible

    wxString message;

    // Ensure that the same begin position (in the underlying audio/video data, without speedup) can be used after changing the speed.
    pts begin = model::Convert::positionToNormalSpeed(clipInterval->getOffset(), clipInterval->getSpeed());
    model::TransitionPtr inTransition = clipInterval->getInTransition();
    if (inTransition && inTransition->getRight() > 0)
    {
        // Ensure there is still enough room for the transition frames (start at same frame)
        //                                                                            render 
        // 0                     begin                                        end     length
        // |--------|--------------|-------------------------------------------|--------|
        // |        |//////////////|                                           |        |
        // |        |//TRANSITION//|        SHOWN CLIP INTERVAL                |        |
        // |        |//////////////|                                           |        |
        // |--------|--------------|-------------------------------------------|--------|
        //           <    right   > <              length                     >
        //  
        // begin / max_speed >= right    ====>    max_speed =< begin / right
        rational maxSpeed{ rational(begin, inTransition->getLength()) };
        if (mSpeed > maxSpeed)
        {
            message = _("Can't scale beyond " + toString(maxSpeed) + " (no room for left transition).");
            mSpeed = maxSpeed;
        }
    }

    // Ensure that the same end position (in the underlying audio/video data, without speedup) can be used after changing the speed.
    pts end = model::Convert::positionToNormalSpeed(clipInterval->getOffset() + clipInterval->getLength(), clipInterval->getSpeed());
    model::TransitionPtr outTransition = clipInterval->getOutTransition();
    if (outTransition && outTransition->getLeft() > 0)
    {
        // Ensure there is still enough room for the transition frames (end at same frame)
        //                                                                               render 
        //            begin                                        end                   length
        // |------------|-------------------------------------------|--------------|--------|
        // |            |                                           |//////////////|        |
        // |            |        SHOWN CLIP INTERVAL                |//TRANSITION//|        |
        // |            |                                           |//////////////|        |
        // |------------|-------------------------------------------|--------------|--------|
        //  <  offset  > <              length                     > <    left   >
        // 
        // (renderlength - end) / max_speed >= left    ====>    max_speed =< (renderlength - end) / left
        rational maxSpeed{ rational(clipInterval->getRenderSourceLength() - end, outTransition->getLength()) }; // todo replace speed with rational<int64>?
        if (mSpeed > maxSpeed)
        {
            message = _("Can't scale beyond " + toString(maxSpeed) + " (no room for right transition).");
            mSpeed = maxSpeed;
        }
    }

    // Ensure that there's always a difference between the begin and end points
    // (end - begin) / maxspeed >= 1
    rational maxSpeed{ rational(end - 1 - begin, 1) };
    if (mSpeed > maxSpeed)
    {
        message = _("Can't scale beyond " + toString(maxSpeed) + " (no clip remains).");
        mSpeed = maxSpeed;
    }
    
    if (!message.IsEmpty())
    {
        gui::StatusBar::get().timedInfoText(message);
    }
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, const EditClipSpeed& obj)
{
    os << static_cast<const AClipEdit&>(obj) << '|' << obj.mClip << '|' << obj.mLink << '|' << obj.mSpeed;
    return os;
}

}}} // namespace