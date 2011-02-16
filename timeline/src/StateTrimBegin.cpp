#include "StateTrimBegin.h"

#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/cmdproc.h>
#include <boost/make_shared.hpp>
#include "StateIdle.h"
#include "UtilLog.h"
#include "Tooltip.h"
#include "Clip.h"
#include "UtilCloneable.h"
#include "MousePointer.h"
#include "PositionInfo.h"
#include "GuiPlayer.h"
#include "EditDisplay.h"
#include "VideoClip.h"
#include "ClipView.h"
#include "Project.h"
#include "Zoom.h"
#include "Timeline.h"
#include "TrimBegin.h"

namespace gui { namespace timeline { namespace state {

const wxString sTooltip = _(
    "Move the cursor to change the entry point of the clip.");

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

TrimBegin::TrimBegin( my_context ctx ) // entry
    :   TimeLineState( ctx )
    ,   mStartPosition(0,0)
    ,   mEdit(0)
    ,   mOriginalClip()
    ,   mMustUndo(false)
{
    LOG_DEBUG; 

    const EvLeftDown* event = dynamic_cast<const EvLeftDown*>(triggering_event());
    ASSERT(event); // Only way to get here is to press left button in the Idle state

    mStartPosition = event->mPosition;
    mCurrentPosition = mStartPosition;
    PointerPositionInfo info = getMousePointer().getInfo(mCurrentPosition);
    mOriginalClip = info.clip;

    ClipView::holdThumbnails();

    mEdit = getPlayer()->startEdit();
    show();
}

TrimBegin::~TrimBegin() // exit
{
    ClipView::releaseThumbnails();
    getPlayer()->endEdit();
    LOG_DEBUG; 
}
//////////////////////////////////////////////////////////////////////////
// EVENTS
//////////////////////////////////////////////////////////////////////////

boost::statechart::result TrimBegin::react( const EvLeftUp& evt )
{
    VAR_DEBUG(evt);
    return transit<Idle>();
}

boost::statechart::result TrimBegin::react( const EvMotion& evt )
{
    VAR_DEBUG(evt);
    mCurrentPosition = evt.mPosition;
    show();
    return discard_event();
}

boost::statechart::result TrimBegin::react( const EvKeyDown& evt)
{
    VAR_DEBUG(evt);
    switch (evt.mWxEvent.GetKeyCode())
    {
    case WXK_F1:
        getTooltip().show(sTooltip);
        break;
    }
    return discard_event();
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

pts TrimBegin::getDiff()
{
    int diff = mCurrentPosition.x - mStartPosition.x;
    pts diff_pts = getZoom().pixelsToPts(diff);
    if (diff_pts > 0)
    {   
        // Move to the right: the clip is shortened
        if (diff_pts >= mOriginalClip->getNumberOfFrames() )
        {
            diff_pts = mOriginalClip->getNumberOfFrames() - 1; // -1: Ensure that resulting clip has always minimally one frame left
        }
    }
    else if (diff_pts < 0)
    {
        // Move to the left: the clip is enlarged
        if (mOriginalClip->getOffset() + diff_pts <= 1)
        {
            diff_pts = -1 * (mOriginalClip->getOffset() - 1); // -1: Ensure that resulting clip has always minimally one frame left
        }



        //@todo if moving to left, in newly used area there may only be empty space in case of 'not shift'
    }
    else
    {
        // At original position;
    }
    // @todo make clip longer
    return diff_pts;
}

model::ClipPtr TrimBegin::getUpdatedClip()
{
    model::ClipPtr clip = make_cloned<model::Clip>(mOriginalClip);
    clip->adjustBegin(getDiff());
    return clip;
}

void TrimBegin::show()
{
    model::ClipPtr updatedClip = getUpdatedClip();
    if (updatedClip->isA<model::VideoClip>())
    { 
        model::VideoClipPtr videoclip = boost::dynamic_pointer_cast<model::VideoClip>(updatedClip);
        VAR_DEBUG(*mOriginalClip)(*updatedClip);
        videoclip->moveTo(0);
        //VAR_DEBUG(*mOriginalClip)(*updatedClip);
        wxSize s = mEdit->getSize();
        model::VideoFramePtr videoFrame = videoclip->getNextVideo(s.GetWidth(), s.GetHeight(), false);
        boost::shared_ptr<wxBitmap> bmp = boost::make_shared<wxBitmap>(wxBitmap(wxImage(videoFrame->getWidth(), videoFrame->getHeight(), videoFrame->getData()[0], true)));
        mEdit->show(bmp);
    }
    bool toLeft = updatedClip->getOffset() < mOriginalClip->getOffset();
    bool toRight = updatedClip->getOffset() > mOriginalClip->getOffset();


    if (mMustUndo)
    {
        model::Project::current()->GetCommandProcessor()->Undo();
    }
    if (toLeft || toRight)
    {
        model::Project::current()->Submit(new command::TrimBegin(getTimeline(), mOriginalClip, getDiff()));
        mMustUndo = true;
        getTimeline().Refresh(false);
getTimeline().Update();
    }
}

}}} // namespace