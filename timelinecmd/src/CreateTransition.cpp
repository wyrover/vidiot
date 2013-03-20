#include "CreateTransition.h"

#include "AudioTransition_CrossFade.h"
#include "Config.h"
#include "EmptyClip.h"
#include "IClip.h"
#include "MousePointer.h"
#include "Timeline.h"
#include "Track.h"
#include "Transition.h"
#include "UtilLog.h"
#include "UtilLogStl.h"
#include "VideoClip.h"
#include "UtilLogWxwidgets.h"
#include "VideoTransition.h"
#include "VideoTransition_CrossFade.h"

namespace gui { namespace timeline { namespace command {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

CreateTransition::CreateTransition(model::SequencePtr sequence, model::IClipPtr clip, model::TransitionPtr transition, model::TransitionType type)
:   AClipEdit(sequence)
,   mTransition(transition)
,   mLeft()
,   mRight()
,   mLeftSize(0)
,   mRightSize(0)
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
    if (mLeft)
    {
        mLeftSize = defaultSize / 2; // Default length // todo do we want to use the defaultSize if the transition is a fade in or fade out?
        mLeftSize = std::min( mLeftSize, -1 * mLeft->getMinAdjustEnd() ); // -1 * : getMinAdjustEnd() <= 0
        if (mRight)
        {
            mLeftSize = std::min( mLeftSize, -1 * mRight->getMinAdjustBegin() ); // -1 * : getMinAdjustBegin() <= 0
        }
    }
    if (mRight)
    {
        mRightSize = defaultSize / 2; // Default length
        mRightSize = std::min( mRightSize, mRight->getMaxAdjustBegin() );
        if (mLeft)
        {
            mRightSize = std::min( mRightSize, mLeft->getMaxAdjustEnd() );
        }
    }
    ASSERT_LESS_THAN_EQUALS(mLeftSize + mRightSize,defaultSize);
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
    return mLeftSize != 0 || mRightSize != 0;
}

model::IClipPtr CreateTransition::getLeftClip() const
{
    return mLeft;
}

model::IClipPtr CreateTransition::getRightClip() const
{
    return mRight;
}

pts CreateTransition::getLeftSize() const
{
    return mLeftSize;
}

pts CreateTransition::getRightSize() const
{
    return mRightSize;
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<( std::ostream& os, const CreateTransition& obj )
{
    os << static_cast<const AClipEdit&>(obj) << '|' << obj.mLeft << '|' << obj.mRight << '|' << obj.mLeftSize << '|' << obj.mRightSize;
    return os;
}

}}} // namespace