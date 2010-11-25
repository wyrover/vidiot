#include "ClipView.h"

#include <wx/dcclient.h>
#include <wx/dcmemory.h>
#include <wx/pen.h>
#include <boost/foreach.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include "Zoom.h"
#include "Constants.h"
#include "UtilLog.h"
#include "AProjectViewNode.h"
#include "VideoFile.h"
#include "VideoClip.h"
#include "EmptyClip.h"
#include "TrackView.h"
#include "Timeline.h"
#include "Selection.h"
#include "ViewMap.h"
#include "Track.h"

namespace gui { namespace timeline {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

ClipView::ClipView(model::ClipPtr clip, View* parent)
:   View(parent)
,   mClip(clip)
,   mThumbnail()
,   mRect(0,0,0,0)
{
    ASSERT(mClip);
}

void ClipView::init()
{
    getViewMap().registerView(mClip,this);
    updateThumbnail();
}

ClipView::~ClipView()
{
    getViewMap().unregisterView(mClip);
}

//////////////////////////////////////////////////////////////////////////
// CONVERSION BETWEEN MODEL AND VIEW
//////////////////////////////////////////////////////////////////////////

model::ClipPtr ClipView::getClip()
{
    return mClip;
}

//////////////////////////////////////////////////////////////////////////
//  GET & SET
//////////////////////////////////////////////////////////////////////////

boost::int64_t ClipView::getLeftPosition() const
{
    return getZoom().ptsToPixels(mClip->getLeftPts());
}
boost::int64_t ClipView::getRightPosition() const
{
    return getZoom().ptsToPixels(mClip->getRightPts());
}

void ClipView::show(wxRect rect)
{
    mRect.width = rect.width;
    mRect.x = rect.x;
    mRect.y = 4;
    mRect.height = requiredHeight() - 8; 
    invalidateBitmap();;
}

int ClipView::requiredWidth()
{
    return getRightPosition() - getLeftPosition();
}

int ClipView::requiredHeight()
{
    return mClip->getTrack()->getHeight();
}

void ClipView::updateThumbnail()
{
    model::VideoClipPtr videoclip = boost::dynamic_pointer_cast<model::VideoClip>(mClip);
    if (videoclip)
    {
        mClip->moveTo(0);
        model::VideoFramePtr videoFrame = videoclip->getNextVideo(requiredWidth() - 2 * Constants::sClipBorderSize, requiredHeight() - 2 * Constants::sClipBorderSize, false);
        mThumbnail.reset(new wxBitmap(wxImage(videoFrame->getWidth(), videoFrame->getHeight(), videoFrame->getData()[0], true)));
        mClip->moveTo(0);
    }
    invalidateBitmap();
}


void ClipView::draw(wxBitmap& bitmap)
{
    wxMemoryDC dc(bitmap);

    if (mClip->isA<model::EmptyClip>())
    {
        dc.SetBrush(Constants::sBackgroundBrush);
        dc.SetPen(Constants::sBackgroundPen);
        dc.DrawRectangle(0,0,requiredWidth(),requiredHeight());
    }
    else
    {
        if (getSelection().isSelected(mClip))
        {
            dc.SetBrush(Constants::sSelectedClipBrush);
            dc.SetPen(Constants::sSelectedClipPen);
        }
        else
        {
            dc.SetBrush(Constants::sClipBrush);
            dc.SetPen(Constants::sClipPen);
        }
        dc.DrawRectangle(0,0,requiredWidth(),requiredHeight());
        if (mThumbnail)
        {
            dc.DrawBitmap(*mThumbnail,wxPoint(Constants::sClipBorderSize,Constants::sClipBorderSize));
        }
    }

    if (mRect.GetHeight() != 0)
    {
        dc.SetBrush(*wxTRANSPARENT_BRUSH);
        dc.SetPen(*wxGREEN_PEN);
        dc.DrawRectangle(mRect);
    }
}

}} // namespace
