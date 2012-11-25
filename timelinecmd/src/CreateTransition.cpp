#include "CreateTransition.h"

#include "AudioTransition_CrossFade.h"
#include "Config.h"
#include "EmptyClip.h"
#include "IClip.h"
#include "MousePointer.h"
#include "PositionInfo.h"
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

CreateTransition::CreateTransition(model::SequencePtr sequence, wxPoint position, model::TransitionPtr transition)
:   AClipEdit(sequence)
,   mTransition(transition)
,   mLeft()
,   mRight()
,   mLeftSize(0)
,   mRightSize(0)
{
    VAR_INFO(this)(position)(*transition);
    mCommandName = _("Create transition");

    pts defaultSize = Config::ReadLong(Config::sPathDefaultTransitionLength);

    PointerPositionInfo info = getTimeline().getMousePointer().getInfo(position);
    if (info.clip && !info.clip->isA<model::EmptyClip>() && !info.clip->isA<model::Transition>())
    {
        switch (info.logicalclipposition)
        {
        case ClipBegin:
            mLeft = info.clip->getPrev();
            mRight = info.clip;
            break;
        case ClipInterior:
            break;
        case ClipEnd:
            mLeft = info.clip;
            mRight = info.clip->getNext();
            break;
        default:
            FATAL("Unexpected logical clip position.");
        }
    }

    if  (mLeft)
    {
        if (mLeft->isA<model::EmptyClip>() || mLeft->isA<model::Transition>())
        {
            mLeft.reset();
        }
    }
    if  (mRight)
    {
        if (mRight->isA<model::EmptyClip>() || mRight->isA<model::Transition>())
        {
            mRight.reset();
        }
    }

    if (mLeft)
    {
        mLeftSize = defaultSize / 2; // Default length
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

void CreateTransition::initialize()
{
    VAR_INFO(this);
    ASSERT(isPossible());

    mTransition->init(mLeftSize,mRightSize);
    addTransition(mLeft,mRight,mTransition);
}

bool CreateTransition::isPossible()
{
    return mLeftSize != 0 || mRightSize != 0;
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