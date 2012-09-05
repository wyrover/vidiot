#include "Trim.h"

#include "Clip.h"
#include "ClipView.h"
#include "EmptyClip.h"
#include "Layout.h"
#include "MousePointer.h"
#include "Player.h"
#include "PositionInfo.h"
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
#include "VideoFrame.h"
#include "VideoCompositionParameters.h"
#include "Zoom.h"

namespace gui { namespace timeline {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

Trim::Trim(Timeline* timeline)
    :   Part(timeline)
    ,   mActive(true)
    ,   mStartPosition(0,0)
    ,   mFixedPixel(0)
    ,   mCommand(0)
    ,   mDc()
{
    VAR_DEBUG(this);
}

Trim::~Trim()
{
    VAR_DEBUG(this);
}

//////////////////////////////////////////////////////////////////////////
// TRIM OPERATION
//////////////////////////////////////////////////////////////////////////

void Trim::start()
{
    LOG_DEBUG;

    // Reset first
    mActive = true;
    mStartPosition = wxPoint(0,0);
    mFixedPixel = 0;

    // Determine if pointer was at begin or at end of clip
    wxPoint virtualMousePosition = getMousePointer().getLeftDownPosition();
    PointerPositionInfo info = getMousePointer().getInfo(virtualMousePosition);
    ASSERT(info.clip && !info.clip->isA<model::EmptyClip>())(info);
    model::TransitionPtr transition = boost::dynamic_pointer_cast<model::Transition>(info.clip);
    MouseOnClipPosition mPosition = info.logicalclipposition;

    // Start position is the physical position of the mouse within the timeline
    getTimeline().CalcScrolledPosition(virtualMousePosition.x,virtualMousePosition.y,&mStartPosition.x,&mStartPosition.y);

    model::IClipPtr originalClip;
    model::IClipPtr adjacentClip;
    bool isBeginTrim = true;
    switch (mPosition)
    {
    case ClipBegin:
        ASSERT(!transition);
        originalClip = info.clip;
        mFixedPts = originalClip->getRightPts(); // Do not optimize away (using ->getRightPts() in the calculation. Since the scrolling is changed and clips are added/removed, that's very volatile information).
        adjacentClip = originalClip->getPrev();
        if (adjacentClip)
        {
            adjacentClip->moveTo(adjacentClip->getLength() - 1);
        }
        break;
    case ClipEnd:
        ASSERT(!transition);
        originalClip = info.clip;
        mFixedPts = originalClip->getLeftPts(); // Do not optimize away (using ->getLeftPts() in the calculation. Since the scrolling is changed and clips are added/removed, that's very volatile information).
        adjacentClip = originalClip->getNext();
        if (adjacentClip)
        {
            adjacentClip->moveTo(0);
        }
        isBeginTrim = false;
        break;
    case TransitionRightClipBegin:
        ASSERT(transition);
        ASSERT_MORE_THAN_ZERO(transition->getRight());
        originalClip = info.clip->getNext();
        mFixedPts = originalClip->getRightPts(); // Do not optimize away (using ->getRightPts() in the calculation. Since the scrolling is changed and clips are added/removed, that's very volatile information).
        if (transition->getLeft() > 0)
        {
            adjacentClip = make_cloned<model::IClip>(transition->getPrev());
            ASSERT(adjacentClip);
            adjacentClip->adjustBegin(adjacentClip->getLength());
            adjacentClip->adjustEnd(transition->getLeft());
            adjacentClip->moveTo(adjacentClip->getLength() - 1);
        }
        break;
    case TransitionLeftClipEnd:
        ASSERT(transition);
        ASSERT_MORE_THAN_ZERO(transition->getLeft());
        originalClip = info.clip->getPrev();
        mFixedPts = originalClip->getLeftPts(); // Do not optimize away (using ->getLeftPts() in the calculation. Since the scrolling is changed and clips are added/removed, that's very volatile information).
        if (transition->getRight() > 0)
        {
            adjacentClip = make_cloned<model::IClip>(transition->getNext());
            ASSERT(adjacentClip);
            adjacentClip->adjustEnd(- adjacentClip->getLength());
            adjacentClip->adjustBegin(-transition->getRight());
            adjacentClip->moveTo(0);
        }
        isBeginTrim = false;
        break;
    case TransitionBegin:
        ASSERT(transition);
        originalClip = info.clip;
        transition.reset();
        break;
    case TransitionEnd:
        ASSERT(transition);
        originalClip = info.clip;
        transition.reset();
        isBeginTrim = false;
        break;
    case ClipInterior:
    case TransitionLeftClipInterior:
    case TransitionInterior:
    case TransitionRightClipInterior:
    default:
        FATAL("Illegal clip position");
    }
    ASSERT(originalClip);

    mFixedPixel = getScrolling().ptsToPixel(mFixedPts); // See remark above.
    if (adjacentClip && (adjacentClip->isA<model::IVideo>()))
    {
        // This applies to both VideoClip and EmptyClip
        model::IVideoPtr adjacentvideoclip = boost::dynamic_pointer_cast<model::IVideo>(adjacentClip);
        model::VideoFramePtr adjacentFrame = adjacentvideoclip->getNextVideo(model::VideoCompositionParameters().setBoundingBox(wxSize(getPlayer()->getVideoSize().GetWidth() / 2,  getPlayer()->getVideoSize().GetHeight())));
        mAdjacentBitmap = adjacentFrame->getBitmap();
    }

    // Prepare for previewing
    mPreviewVideoClip = boost::dynamic_pointer_cast<model::VideoClip>(make_cloned<model::IClip>(originalClip));
    if (mPreviewVideoClip)
    {
        wxSize playerSize = getPlayer()->getVideoSize();

        model::TransitionPtr inTransition = originalClip->getInTransition();
        model::TransitionPtr outTransition = originalClip->getOutTransition();

        pts originalOffset = mPreviewVideoClip->getOffset();
        pts originalLength = mPreviewVideoClip->getLength();
        if (isBeginTrim)
        {
            mStartPositionPreview = originalOffset; // Left begin point
            if (inTransition)
            {
                mStartPositionPreview -= inTransition->getRight(); // Since the trim starts on the position where the two transitioned clips 'touched'
            }
        }
        else
        {
            mStartPositionPreview = originalOffset + originalLength - 1; // -1 required since end is 'one too far
            if (outTransition)
            {
                mStartPositionPreview += outTransition->getLeft(); // Since the trim starts on the position where the two transitioned clips 'touched'
            }
        }
        mPreviewVideoClip->maximize();

        mDc.SetBrush(Layout::get().PreviewBackgroundBrush);
        mDc.SetPen(Layout::get().PreviewBackgroundPen);
        mBitmapSingle = boost::make_shared<wxBitmap>(playerSize);
        mDc.SelectObject(*mBitmapSingle);
        mDc.DrawRectangle(wxPoint(0,0),mDc.GetSize());
        if (mAdjacentBitmap)
        {
            mBitmapSideBySide = boost::make_shared<wxBitmap>(playerSize);
            mDc.SelectObject(*mBitmapSideBySide);
            int xAdjacent = (isBeginTrim ? 0 : playerSize.GetWidth() / 2);
            mDc.DrawBitmap(*mAdjacentBitmap, xAdjacent, (playerSize.GetHeight() - mAdjacentBitmap->GetHeight()) / 2);
        }
        mDc.SelectObject(wxNullBitmap);
    }

    mCommand = new command::TrimClip(getSequence(), originalClip, transition, mPosition);
    mCommand->update(0);

    preview();
}

void Trim::update(wxPoint position)
{
    VAR_DEBUG(this);
    getTimeline().beginTransaction();

    pts diff = getZoom().pixelsToPts(position.x - mStartPosition.x);
    mCommand->update(diff);
    preview();

    //wxPoint pos(mCommand->getShiftStart(),0);
    //wxSize s(wxSize(mCommand->getDiff(),getTimeline().GetSize().GetHeight()));
    //getTimeline().RefreshRect(wxRect(pos,s), false);

    // todo bug: shift drag at begin of clip, clip is preceded by empty clip -> preview still shows another clip??

    getTimeline().endTransaction();
    getTimeline().Update();

    if (wxGetMouseState().ShiftDown() && mCommand->isBeginTrim())
    {
        // Ensure that the rightmost pts is kept at the same position when shift dragging

        // this caused one of the automated test scenarios to fail (the scroll change causes an error in the trim code)
        //getScrolling().align(mFixedPts - diff, mFixedPixel);// TODO improve: the feedback looks crappy and sometimes the alignment is incorrect
        // idea: do not change scrolling but just shift the bitmap a bit during the edit operation.
        // do the scroll adjust at the end of the edit operation?
    }
}

void Trim::abort()
{
    // todo rename into end...or stop. This is always called when exiting statetrim
    VAR_DEBUG(this);
    mActive = false;
    if (mCommand)
    {
        delete mCommand;
        mCommand = 0;
        getTimeline().Update();
    }
}

void Trim::stop()
{
    // todo rename in submit...
    VAR_DEBUG(this);
    if (mCommand->getDiff() != 0)
    {
        // Only submit the command if there's an actual diff to be applied
        mCommand->submit();
        mCommand = 0; // To ensure that any following 'abort' (see StateTrim) will not cause a revert
        LOG_DEBUG;
    }
}

//////////////////////////////////////////////////////////////////////////
// DRAW
//////////////////////////////////////////////////////////////////////////

void Trim::draw(wxDC& dc) const
{
    return;
    if (!mActive)
    {
        return;
    }

    if (!wxGetMouseState().ShiftDown())
    {
        return;
    }

    dc.SetPen(Layout::get().SnapPen);
    dc.SetBrush(Layout::get().SelectedClipBrush);
    wxPoint pos(getZoom().ptsToPixels(mCommand->getShiftStart()),0);
    wxSize s(wxSize(getZoom().ptsToPixels(mCommand->getDiff()),dc.GetSize().GetHeight()));
    dc.DrawRectangle(pos,s);
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void Trim::preview()
{
    if (!mPreviewVideoClip) { return; }

    pts diff = mCommand->getDiff();
    pts position(mStartPositionPreview + diff);
    wxSize playerSize = getPlayer()->getVideoSize();
    bool isBeginTrim = mCommand->isBeginTrim();
    bool drawSideBySide = wxGetMouseState().ShiftDown() && mAdjacentBitmap;

    bool completelyTrimmedAway = false;
    ASSERT_LESS_THAN_EQUALS(mStartPositionPreview + diff,mPreviewVideoClip->getLength());
    if (position == mPreviewVideoClip->getLength())
    {
        ASSERT(isBeginTrim);
        completelyTrimmedAway = true; // Clip has been trimmed away completely. From the beginning.
    }
    ASSERT_MORE_THAN_EQUALS(position,-1);
    if (position == -1)
    {
        ASSERT(!isBeginTrim);
        completelyTrimmedAway = true; // Clip has been trimmed away completely. From the end.
    }

    pixel w = playerSize.GetWidth();
    pixel h = playerSize.GetHeight();
    wxBitmapPtr bitmap = mBitmapSingle;
    if (drawSideBySide)
    {
        w = w / 2;
        bitmap = mBitmapSideBySide;
    }
    int x = isBeginTrim ? playerSize.GetWidth() - w : 0;

    mDc.SelectObject(*bitmap);
    if (completelyTrimmedAway)
    {
        mDc.SetBrush(Layout::get().PreviewBackgroundBrush);
        mDc.SetPen(Layout::get().PreviewBackgroundPen);
        mDc.DrawRectangle(x, 0, w, h);
    }
    else
    {
        mPreviewVideoClip->moveTo(position);
        model::VideoFramePtr videoFrame = mPreviewVideoClip->getNextVideo(model::VideoCompositionParameters().setBoundingBox(wxSize(w, h)));
        ASSERT(videoFrame); // A frame must be possible, due to the 'completelyTrimmedAway' check above.
        wxBitmapPtr trimmedBmp = videoFrame->getBitmap();
        ASSERT(trimmedBmp);
        mDc.DrawBitmap(*trimmedBmp, x, (h - trimmedBmp->GetHeight()) / 2);
    }
    mDc.SelectObject(wxNullBitmap);
    getPlayer()->show(bitmap);
}

}} // namespace