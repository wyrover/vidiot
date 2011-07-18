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

CreateTransition::CreateTransition(gui::timeline::Timeline& timeline, wxPoint position)
:   AClipEdit(timeline)
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
            mLeft = info.track->getPreviousClip(info.clip);
            mRight = info.clip;
            break;
        case ClipInterior:
            break;
        case ClipEnd:
            mLeft = info.clip;
            mRight = info.track->getNextClip(info.clip);
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

    model::TrackPtr track;
    model::IClipPtr position;

    model::IClipPtr transitionLeftClip;
    model::IClipPtr transitionRightClip;
    
    if (mLeftSize > 0)
    {
        // Determine position of transition
        track = mLeft->getTrack();
        position = track->getNextClip(mLeft);

        // Determine adjustment and adjust clip
        model::IClipPtr updatedLeft = make_cloned<model::IClip>(mLeft);
        pts adjustment = -mLeftSize;
        adjustment = std::max( adjustment, updatedLeft->getMinAdjustEnd() );
        adjustment = std::min( adjustment, updatedLeft->getMaxAdjustEnd() );
        updatedLeft->adjustEnd(adjustment);
        replaceClip(mLeft,boost::assign::list_of(updatedLeft),&linkmapper);

        // Make copy of left clip for the transition
        transitionLeftClip = make_cloned<model::IClip>(mLeft);
        transitionLeftClip->adjustBegin(transitionLeftClip->getLength() - mLeftSize);
        transitionLeftClip->adjustEnd(mRightSize);
    }
    if (mRightSize > 0)
    {
        // Determine position of transition
        track = mRight->getTrack();

        // Determine adjustment and adjust clip
        model::IClipPtr updatedRight = make_cloned<model::IClip>(mRight);
        pts adjustment = mRightSize;
        adjustment = std::max( adjustment, updatedRight->getMinAdjustBegin() );
        adjustment = std::min( adjustment, updatedRight->getMaxAdjustBegin() );
        updatedRight->adjustBegin(adjustment);
        replaceClip(mRight,boost::assign::list_of(updatedRight),&linkmapper);
        
        // Make copy of right clip for the transition
        transitionRightClip = make_cloned<model::IClip>(mRight);
        transitionRightClip->adjustEnd(mRightSize - transitionRightClip->getLength());
        transitionRightClip->adjustBegin(-mLeftSize);

        // Determine position of transition
        position = updatedRight;
    }
    ASSERT(track);
    ASSERT(position);
    model::IClipPtr transition = boost::make_shared<model::transition::CrossFade>(transitionLeftClip, mLeftSize, transitionRightClip, mRightSize); 
    newMove(track,position,boost::assign::list_of(transition));

    LOG_DEBUG << "STEP 3: Ensure that links are maintained.";
    replaceLinks(linkmapper);
}

bool CreateTransition::isPossible()
{
    return mLeftSize != 0 || mRightSize != 0;
}

}}} // namespace

