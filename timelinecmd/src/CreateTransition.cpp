#include "CreateTransition.h"

#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>
#include <boost/assign/list_of.hpp>
#include "CrossFade.h"
#include "EmptyClip.h"
#include "IClip.h"
#include "MousePointer.h"
#include "PositionInfo.h"
#include "Timeline.h"
#include "Track.h"
#include "UtilLog.h"
#include "UtilLogStl.h"
#include "UtilLogWxwidgets.h"
#include "VideoTransition.h"

namespace gui { namespace timeline { namespace command {

    static const int sDefaultTransitionSize = 24;

CreateTransition::CreateTransition(model::SequencePtr sequence, wxPoint position)
:   AClipEdit(sequence)
,   mLeft()
,   mRight()
,   mLeftSize(0)
,   mRightSize(0)
{
    VAR_INFO(this)(position);
    mCommandName = _("Create transition");

    PointerPositionInfo info = getTimeline().getMousePointer().getInfo(position);
    if (info.clip && !info.clip->isA<model::EmptyClip>())
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
    if (mLeft && !mLeft->isA<model::EmptyClip>())
    {
        if (mRight && !mRight->isA<model::EmptyClip>())
        {
            mLeftSize = -1 * mRight->getMinAdjustBegin();
        }
        else
        {
            mLeftSize = mLeft->getLength();
        }
    }
    if (mRight && !mRight->isA<model::EmptyClip>())
    {
        if (mLeft && !mLeft->isA<model::EmptyClip>())
        {
            mRightSize  = mLeft->getMaxAdjustEnd();
        }
        else
        {
            mRightSize = mRight->getLength();
        }
    }
    if (mLeftSize > 0 && mRightSize > 0)
    {
        if (mLeftSize + mRightSize > sDefaultTransitionSize)
        {
            if (mLeftSize > sDefaultTransitionSize / 2)
            {
                mLeftSize = sDefaultTransitionSize / 2;
            }
            if (mRightSize > sDefaultTransitionSize / 2)
            {
                mRightSize = sDefaultTransitionSize / 2;
            }
        }
    }
    else if (mLeftSize > 0)
    {
        mLeftSize = sDefaultTransitionSize;
    }
    else if (mRightSize > 0)
    {
        mRightSize = sDefaultTransitionSize;
    }
}

CreateTransition::~CreateTransition()
{
}

void CreateTransition::initialize()
{
    VAR_INFO(this);
    ASSERT(isPossible());

    ReplacementMap linkmapper;

    AClipEdit::makeTransition(mLeft,mLeftSize,mRight,mRightSize,linkmapper);

    replaceLinks(linkmapper);
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

