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
    ,   mFixedPixel(0)
    ,   mCommand(0)
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

    mEdit = getPlayer()->startEdit();

    // Reset first
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
        break;
    case TransitionRightClipBegin:
        ASSERT(transition);
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
        break;
    case TransitionBegin:
        ASSERT(transition);
        originalClip = info.clip;
        //break;
    case TransitionEnd:
        ASSERT(transition);
        originalClip = info.clip;
        // break;
    case ClipInterior:
    case TransitionLeftClipInterior:
    case TransitionInterior:
    case TransitionRightClipInterior:
    default:
        FATAL("Illegal clip position");
    }
    ASSERT(originalClip);

    // \todo use the leftmost of the clip and/or its link

    mFixedPixel = getScrolling().ptsToPixel(mFixedPts); // See remark above.
    if (adjacentClip && adjacentClip->isA<model::VideoClip>())
    {
        model::VideoClipPtr adjacentvideoclip = boost::dynamic_pointer_cast<model::VideoClip>(adjacentClip);
        model::VideoFramePtr adjacentFrame = adjacentvideoclip->getNextVideo(wxSize(mEdit->getSize().GetWidth() / 2,  mEdit->getSize().GetHeight()), false);
        mAdjacentBitmap = adjacentFrame->getBitmap();
    }

    mCommand = new command::TrimClip(getSequence(), originalClip, transition, mPosition);
    mCommand->update(0);
    preview();
}

void Trim::update(wxPoint position)
{
    getTimeline().beginTransaction();

    pts diff = getZoom().pixelsToPts(position.x - mStartPosition.x);
    mCommand->update(diff);
    preview();

    getTimeline().endTransaction();
    getTimeline().Update();

    if (wxGetMouseState().ShiftDown() && mCommand->isBeginTrim())
    {
        // Ensure that the rightmost pts is kept at the same position when shift dragging

        // this caused one of the automated test scenarios to fail (the scroll change causes an error in the trim code)
   //     getScrolling().align(mFixedPts - diff, mFixedPixel);// TODO improve: the feedback looks crappy and sometimes the alignment is incorrect
        // idea: do not change scrolling but just shift the bitmap a bit during the edit operation.
        // do the scroll adjust at the end of the edit operation?
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
        mCommand = 0; // To ensure that any following 'abort' (see StateTrim) will not cause a revert
        LOG_DEBUG;
    }

    getPlayer()->endEdit();
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void Trim::preview()
{
    model::IClipPtr updatedClip = make_cloned<model::IClip>(mCommand->getClip());

    if (updatedClip->isA<model::VideoClip>())
    {
        if (mCommand->isBeginTrim())
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
            bool drawadjacentclip = wxGetMouseState().ShiftDown() && mAdjacentBitmap;
            int previewwidth = (drawadjacentclip ? s.GetWidth() / 2 : s.GetWidth());
            int previewxpos = 0;
            boost::shared_ptr<wxBitmap> bmp = boost::make_shared<wxBitmap>(s);
            wxMemoryDC dc(*bmp);

            if (mCommand->isBeginTrim())
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
                int xAdjacent = (mCommand->isBeginTrim() ? 0 : s.GetWidth() / 2);
                dc.DrawBitmap(*mAdjacentBitmap, xAdjacent, (s.GetHeight() - mAdjacentBitmap->GetHeight()) / 2);
            }
            dc.SelectObject(wxNullBitmap);
            mEdit->show(bmp);
        }
    }
}

}} // namespace