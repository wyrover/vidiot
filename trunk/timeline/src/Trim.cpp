#include "Trim.h"

#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/cmdproc.h>
#include <wx/dcmemory.h>
#include <wx/wupdlock.h>
#include <boost/foreach.hpp>
#include <boost/limits.hpp>
#include <boost/make_shared.hpp>
#include "Clip.h"
#include "ClipView.h"
#include "EditDisplay.h"
#include "EmptyClip.h"
#include "Layout.h"
#include "MousePointer.h"
#include "Player.h"
#include "PositionInfo.h"
#include "Project.h"
#include "Scrolling.h"
#include "Sequence.h"
#include "Timeline.h"
#include "Tooltip.h"
#include "Track.h"
#include "Transition.h"
#include "TrimClip.h"
#include "UtilCloneable.h"
#include "UtilLog.h"
#include "VideoClip.h"
#include "Zoom.h"

namespace gui { namespace timeline {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

Trim::Trim(Timeline* timeline)
    :   Part(timeline)
    ,   mStartPosition(0,0)
    ,   mEdit(0)
    ,   mOriginalClip()
    ,   mShiftDown(false)
    ,   mFixedPixel(0)
    ,   mCommand(0)
{
    VAR_DEBUG(this);
}

Trim::~Trim()
{
    VAR_DEBUG(this);
}

void Trim::start()
{
    LOG_DEBUG;

    mEdit = getPlayer()->startEdit();

    // Reset first
     mStartPosition = wxPoint(0,0);
    mOriginalClip.reset();
    mShiftDown = false;
    mFixedPixel = 0;

    // Determine if pointer was at begin or at end of clip
    wxPoint virtualMousePosition = getMousePointer().getLeftDownPosition();
    PointerPositionInfo info = getMousePointer().getInfo(virtualMousePosition);
    ASSERT(info.clip && !info.clip->isA<model::EmptyClip>())(info);
    mTransition = boost::dynamic_pointer_cast<model::Transition>(info.clip);
    mPosition = info.logicalclipposition;

    // Start position is the physical position of the mouse within the timeline
    getTimeline().CalcScrolledPosition(virtualMousePosition.x,virtualMousePosition.y,&mStartPosition.x,&mStartPosition.y);
    mCurrentPosition = mStartPosition;
    mShiftDown = wxGetMouseState().ShiftDown();

    model::IClipPtr adjacentClip;
    switch (mPosition)
    {
    case ClipBegin:
        ASSERT(!mTransition);
        mTrimBegin = true;
        mOriginalClip = info.clip;
        mFixedPts = mOriginalClip->getRightPts(); // Do not optimize away (using ->getRightPts() in the calculation. Since the scrolling is changed and clips are added/removed, that's very volatile information).
        adjacentClip = mOriginalClip->getPrev();
        if (adjacentClip)
        {
            adjacentClip->moveTo(adjacentClip->getLength() - 1);
        }
        break;
    case ClipEnd:
        ASSERT(!mTransition);
        mTrimBegin = false;
        mOriginalClip = info.clip;
        mFixedPts = mOriginalClip->getLeftPts(); // Do not optimize away (using ->getLeftPts() in the calculation. Since the scrolling is changed and clips are added/removed, that's very volatile information).
        adjacentClip = mOriginalClip->getNext();
        if (adjacentClip)
        {
            adjacentClip->moveTo(0);
        }
        break;
    case TransitionRightClipBegin:
        ASSERT(mTransition);
        mTrimBegin = true;
        mOriginalClip = info.clip->getNext();
        mFixedPts = mOriginalClip->getRightPts(); // Do not optimize away (using ->getRightPts() in the calculation. Since the scrolling is changed and clips are added/removed, that's very volatile information).
        if (mTransition->getLeft() > 0)
        {
            adjacentClip = make_cloned<model::IClip>(mTransition->getPrev());
            ASSERT(adjacentClip);
            adjacentClip->adjustBegin(adjacentClip->getLength());
            adjacentClip->adjustEnd(mTransition->getLeft());
            adjacentClip->moveTo(adjacentClip->getLength() - 1);
        }
        break;
    case TransitionLeftClipEnd:
        ASSERT(mTransition);
        mTrimBegin = false;
        mOriginalClip = info.clip->getPrev();
        mFixedPts = mOriginalClip->getLeftPts(); // Do not optimize away (using ->getLeftPts() in the calculation. Since the scrolling is changed and clips are added/removed, that's very volatile information).
        if (mTransition->getRight() > 0)
        {
            adjacentClip = make_cloned<model::IClip>(mTransition->getNext());
            ASSERT(adjacentClip);
            adjacentClip->adjustEnd(- adjacentClip->getLength());
            adjacentClip->adjustBegin(-mTransition->getRight());
            adjacentClip->moveTo(0);
        }
        break;
    case TransitionBegin:
        ASSERT(mTransition);
        mTrimBegin = true;
        mOriginalClip = info.clip;
        //break;
    case TransitionEnd:
        ASSERT(mTransition);
        mTrimBegin = false;
        mOriginalClip = info.clip;
       // break;
    case ClipInterior:
    case TransitionLeftClipInterior:
    case TransitionInterior:
    case TransitionRightClipInterior:
    default:
        FATAL("Illegal clip position");
    }
    ASSERT(mOriginalClip);

    // \todo use the leftmost of the clip and/or its link

    mFixedPixel = getScrolling().ptsToPixel(mFixedPts); // See remark above.
    if (adjacentClip && adjacentClip->isA<model::VideoClip>())
    {
        model::VideoClipPtr adjacentvideoclip = boost::dynamic_pointer_cast<model::VideoClip>(adjacentClip);
        model::VideoFramePtr adjacentFrame = adjacentvideoclip->getNextVideo(wxSize(mEdit->getSize().GetWidth() / 2,  mEdit->getSize().GetHeight()), false);
        mAdjacentBitmap = adjacentFrame->getBitmap();
    }

    mCommand = new command::TrimClip(getSequence(), mOriginalClip, mTransition, mPosition, mTrimBegin);
    mCommand->update(mShiftDown,0);
    preview();
}

void Trim::update()
{
    getTimeline().beginTransaction();
    // Do not use mOriginalClip: it may have been removed from the track by applying command::TrimClip previously
    // From here we can no longer use mOriginalClip: it is changed by applying the command::TrimClip

    pts diff = getZoom().pixelsToPts(mCurrentPosition.x - mStartPosition.x);
    mCommand->update(mShiftDown,diff);
    preview();

    if (mShiftDown && mTrimBegin)
    {
        // Ensure that the rightmost pts is kept at the same position when shift dragging
        getScrolling().align(mFixedPts - diff, mFixedPixel);
    }
    getTimeline().endTransaction();
    getTimeline().Update();
}

void Trim::onShift(bool shift)
{
        if (mShiftDown != shift)
    {
        mShiftDown = shift;
        update();
    }

}

void Trim::onMove(wxPoint position)
{
 if (mCurrentPosition != position)
    {
        mCurrentPosition = position;
        update();
    }
}

void Trim::abort()
{
    if (mCommand)
    {
        delete mCommand;
        mCommand = 0;
        getTimeline().Update();
    }
}

void Trim::stop()
{
    if (mCommand->getDiff() != 0)
    {
        // Only submit the command if there's an actual diff to be applied
        getTimeline().Submit(mCommand); // todo: test if performance improves by not submitting the command, but only when really submitting, and doing 'local' do/undo otherwise
        getPlayer()->endEdit();
        mCommand = 0; // To ensure that any following 'abort' (see StateTrim) will not cause a revert
        LOG_DEBUG;
    }
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void Trim::preview()
{
    model::IClipPtr updatedClip = make_cloned<model::IClip>(mCommand->getClip());

    if (updatedClip->isA<model::VideoClip>())
    {
        if (mTrimBegin)
        {
            updatedClip->adjustBegin(mCommand->getDiff());
        }
        else
        {
            updatedClip->adjustEnd(mCommand->getDiff());
        }

        if (updatedClip->getLength() > 0)
        {
            model::VideoClipPtr videoclip = boost::dynamic_pointer_cast<model::VideoClip>(updatedClip);
            wxSize s = mEdit->getSize();
            bool drawadjacentclip = mShiftDown && mAdjacentBitmap;
            int previewwidth = (drawadjacentclip ? s.GetWidth() / 2 : s.GetWidth());
            int previewxpos = 0;
            boost::shared_ptr<wxBitmap> bmp = boost::make_shared<wxBitmap>(s);
            wxMemoryDC dc(*bmp);

            if (mTrimBegin)
            {
                videoclip->moveTo(0);
                previewxpos = s.GetWidth() - previewwidth; // This works for both with and without an adjacent clip
            }
            else
            {
                videoclip->moveTo(videoclip->getLength() - 1);
                previewxpos = 0;
            }

            // Fill with black
            dc.SetBrush(Layout::sPreviewBackgroundBrush);
            dc.SetPen(Layout::sPreviewBackgroundPen);
            dc.DrawRectangle(wxPoint(0,0),dc.GetSize());

            // Draw preview of trim operation
            model::VideoFramePtr videoFrame = videoclip->getNextVideo(wxSize(previewwidth, s.GetHeight()), false);
            model::wxBitmapPtr trimmedBmp = videoFrame->getBitmap();
            dc.DrawBitmap(*trimmedBmp, previewxpos, (s.GetHeight() - trimmedBmp->GetHeight()) / 2);

            // Draw adjacent clip if present. Is only relevant when holding shift
            if (drawadjacentclip)
            {
                int xAdjacent = (mTrimBegin ? 0 : s.GetWidth() / 2);
                dc.DrawBitmap(*mAdjacentBitmap, xAdjacent, (s.GetHeight() - mAdjacentBitmap->GetHeight()) / 2);
            }
            dc.SelectObject(wxNullBitmap);
            mEdit->show(bmp);
        }
    }
}

}} // namespace