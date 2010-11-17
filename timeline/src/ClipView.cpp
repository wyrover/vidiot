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

namespace gui { namespace timeline {

DEFINE_EVENT(CLIP_UPDATE_EVENT, ClipUpdateEvent, ClipView*);

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

ClipView::ClipView(TrackView* track,
                                 model::ClipPtr clip)
:   wxWindow(static_cast<wxWindow*>(track),wxID_ANY)
,   mClip(clip)
,   mThumbnail()
,   mWidth(0)
,   mBitmap()
,   mBeingDragged(false)
,   mRect(0,0,0,0)
{
    ASSERT(mClip);
}

void ClipView::init()
{
    getViewMap().registerView(mClip,this);
    updateSize(); // Also creates bitmap

    Hide();
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

const wxBitmap& ClipView::getBitmap()
{
    return mBitmap;
}

void ClipView::setBeingDragged(bool beingdragged)
{
    mBeingDragged = beingdragged;
    // Event is needed to trigger track redraw (without this clip)
    QueueEvent(new ClipUpdateEvent(this));
}

bool ClipView::isBeingDragged()
{
    return mBeingDragged;
}

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
    mRect.height = mBitmap.GetHeight() - 8; 
    updateBitmap();
}

void ClipView::updateSize()
{
    mWidth = getRightPosition() - getLeftPosition();
    mBitmap.Create(mWidth,getViewMap().getView(mClip->getTrack())->getBitmap().GetHeight());
    updateThumbnail();  
}

void ClipView::updateThumbnail()
{
    model::VideoClipPtr videoclip = boost::dynamic_pointer_cast<model::VideoClip>(mClip);
    if (videoclip)
    {
        mClip->moveTo(0);
        model::VideoFramePtr videoFrame = videoclip->getNextVideo(mWidth - 2 * Constants::sClipBorderSize, mBitmap.GetHeight() - 2 * Constants::sClipBorderSize, false);
        mThumbnail.reset(new wxBitmap(wxImage(videoFrame->getWidth(), videoFrame->getHeight(), videoFrame->getData()[0], true)));
        mClip->moveTo(0);
    }
    updateBitmap();
}

void ClipView::updateBitmap()
{
    wxMemoryDC dc(mBitmap);

    if (mClip->isA<model::EmptyClip>())
    {
        dc.SetBrush(Constants::sBackgroundBrush);
        dc.SetPen(Constants::sBackgroundPen);
        dc.DrawRectangle(0,0,mWidth,mBitmap.GetHeight());
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
        dc.DrawRectangle(0,0,mWidth,mBitmap.GetHeight());
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

    QueueEvent(new ClipUpdateEvent(this));
}


}} // namespace
