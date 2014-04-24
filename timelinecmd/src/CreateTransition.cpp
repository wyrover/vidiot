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

#include "CreateTransition.h"

#include "Config.h"
#include "EmptyClip.h"
#include "IClip.h"
#include "Mouse.h"
#include "Timeline.h"
#include "Track.h"
#include "Transition.h"
#include "UtilLog.h"
#include "UtilLogStl.h"
#include "VideoClip.h"
#include "UtilLogWxwidgets.h"
#include "VideoTransition.h"

namespace gui { namespace timeline { namespace command {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

CreateTransition::CreateTransition(const model::SequencePtr& sequence, const model::IClipPtr& clip, const model::TransitionPtr& transition, const model::TransitionType& type)
:   AClipEdit(sequence)
,   mTransition(transition)
,   mType(type)
,   mLeft()
,   mRight()
,   mLeftSize(boost::none)
,   mRightSize(boost::none)
{
    VAR_INFO(this)(clip)(*transition);
    mCommandName = _("Create transition");

    pts defaultSize = Config::ReadLong(Config::sPathDefaultTransitionLength);

    switch (type)
    {
    case model::TransitionTypeIn:                               mRight = clip;              break;
    case model::TransitionTypeOut:   mLeft = clip;                                          break;
    case model::TransitionTypeInOut: mLeft = clip->getPrev();   mRight = clip;              break;
    case model::TransitionTypeOutIn: mLeft = clip;              mRight = clip->getNext();   break;
    default:
        FATAL("Unexpected transition type.");
    }

    if  (mLeft && (mLeft->isA<model::EmptyClip>() || mLeft->isA<model::Transition>()))
    {
        mLeft.reset();
    }
    if  (mRight && (mRight->isA<model::EmptyClip>() || mRight->isA<model::Transition>()))
    {
        mRight.reset();
    }

    ASSERT(mLeft || mRight);
    pts leftSize = 0;
    pts rightSize = 0;
    if (mLeft)
    {
        leftSize = defaultSize / 2; // Default length
        leftSize = std::min( leftSize, -1 * mLeft->getMinAdjustEnd() ); // -1 * : getMinAdjustEnd() <= 0
        if (mRight)
        {
            leftSize = std::min( leftSize, -1 * mRight->getMinAdjustBegin() ); // -1 * : getMinAdjustBegin() <= 0
        }
        mLeftSize.reset(leftSize);
    }
    if (mRight)
    {
        rightSize = defaultSize / 2; // Default length
        rightSize = std::min( rightSize, mRight->getMaxAdjustBegin() );
        if (mLeft)
        {
            rightSize = std::min( rightSize, mLeft->getMaxAdjustEnd() );
        }
        mRightSize.reset(rightSize);
    }
    ASSERT_LESS_THAN_EQUALS(leftSize + rightSize,defaultSize);
// todo for an inout transition that has < defaultsize because one of the sides has not enough room, try taking extra room on the other side
}

CreateTransition::~CreateTransition()
{
}

//////////////////////////////////////////////////////////////////////////
// ACLIPEDIT INTERFACE
//////////////////////////////////////////////////////////////////////////

void CreateTransition::initialize()
{
    VAR_INFO(this);
    ASSERT(isPossible());

    mTransition->init(mLeftSize,mRightSize);
    addTransition(mLeft,mRight,mTransition);
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

bool CreateTransition::isPossible()
{
    switch (mType)
    {
    case model::TransitionTypeIn:    return mRightSize && *mRightSize > 0;
    case model::TransitionTypeOut:   return mLeftSize && *mLeftSize > 0;
    case model::TransitionTypeInOut: // FALLTHROUGH
    case model::TransitionTypeOutIn: 
        // todo (in case only one side has room for transition, do not trim but use only half the transition) return mLeftSize && mRightSize && ((mLeftSize ? *mLeftSize : 0) + (mRightSize ? *mRightSize : 0) > 0);
        return mLeftSize && mRightSize && *mLeftSize > 0 && *mRightSize > 0;
    default:
        FATAL("Unexpected transition type.");
    }
    return false;
}

model::IClipPtr CreateTransition::getLeftClip() const
{
    return mLeft;
}

model::IClipPtr CreateTransition::getRightClip() const
{
    return mRight;
}

boost::optional<pts> CreateTransition::getLeftSize() const
{
    return mLeftSize;
}

boost::optional<pts> CreateTransition::getRightSize() const
{
    return mRightSize;
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, const CreateTransition& obj)
{
    os << static_cast<const AClipEdit&>(obj) << '|' << obj.mLeft << '|' << obj.mRight << '|' << obj.mLeftSize << '|' << obj.mRightSize;
    return os;
}

}}} // namespace