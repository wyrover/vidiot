#include "GuiTimeLineClip.h"

#include <wx/dcclient.h>
#include <wx/dcmemory.h>
#include <wx/pen.h>
#include <boost/foreach.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include "GuiTimeLineZoom.h"
#include "Constants.h"
#include "UtilLog.h"
#include "AProjectViewNode.h"
#include "VideoFile.h"
#include "VideoClip.h"
#include "EmptyClip.h"
#include "GuiTimeLineTrack.h"
#include "GuiTimeLine.h"

namespace gui { namespace timeline {

DEFINE_EVENT(CLIP_UPDATE_EVENT, ClipUpdateEvent, GuiTimeLineClip*);

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

GuiTimeLineClip::GuiTimeLineClip(GuiTimeLine& timeline,
                                 GuiTimeLineTrack* track,
                                 const GuiTimeLineZoom& zoom, 
                                 model::ClipPtr clip)
:   wxWindow(static_cast<wxWindow*>(track),wxID_ANY)
,   mZoom(zoom)
,   mTimeline(timeline)
,   mClip(clip)
,   mThumbnail()
,   mWidth(0)
,   mBitmap()
,   mSelected(false)
,   mBeingDragged(false)
,   mRect(0,0,0,0)
{
    ASSERT(mClip);
    mTimeline.getViewMap().registerView(mClip,this);
    updateSize(); // Also creates bitmap

    Hide();
}

GuiTimeLineClip::~GuiTimeLineClip()
{
    mTimeline.getViewMap().unregisterView(mClip);
}

//////////////////////////////////////////////////////////////////////////
// CONVERSION BETWEEN MODEL AND VIEW
//////////////////////////////////////////////////////////////////////////

model::ClipPtr GuiTimeLineClip::getClip()
{
    return mClip;
}

GuiTimeLineTrack* GuiTimeLineClip::getTrack()
{
    return mTimeline.getViewMap().getView(getClip()->getTrack());
}

//////////////////////////////////////////////////////////////////////////
//  GET & SET
//////////////////////////////////////////////////////////////////////////

const wxBitmap& GuiTimeLineClip::getBitmap()
{
    return mBitmap;
}

bool GuiTimeLineClip::isEmpty() const
{
    return mClip->isA<model::EmptyClip>();
}

bool GuiTimeLineClip::isSelected() const
{
    return mSelected;
}

void GuiTimeLineClip::setSelected(bool selected)
{
    if (mClip->isA<model::EmptyClip>())
    {

    }
    else
    {
        if (selected != mSelected)
        {
            // This if statement is needed to avoid endless 
            // loops when also selecting the linked clip.
            mSelected = selected;
            updateBitmap();
            model::ClipPtr link = mClip->getLink();
            if (link)
            {
                mTimeline.getViewMap().getView(link)->setSelected(selected);
            }
        }
    }
}

void GuiTimeLineClip::setBeingDragged(bool beingdragged)
{
    mBeingDragged = beingdragged;
    // Event is needed to trigger track redraw (without this clip)
    QueueEvent(new ClipUpdateEvent(this));
}

bool GuiTimeLineClip::isBeingDragged()
{
    return mBeingDragged;
}

boost::int64_t GuiTimeLineClip::getLeftPosition() const
{
    return mZoom.ptsToPixels(mClip->getLeftPts());
}
boost::int64_t GuiTimeLineClip::getRightPosition() const
{
    return mZoom.ptsToPixels(mClip->getRightPts());
}

void GuiTimeLineClip::show(wxRect rect)
{
    mRect.width = rect.width;
    mRect.x = rect.x;
    mRect.y = 4;
    mRect.height = mBitmap.GetHeight() - 8; 
    updateBitmap();
}

void GuiTimeLineClip::updateSize()
{
    mWidth = getRightPosition() - getLeftPosition();
    mBitmap.Create(mWidth,mTimeline.getViewMap().getView(mClip->getTrack())->getBitmap().GetHeight());
    updateThumbnail();  
}

void GuiTimeLineClip::updateThumbnail()
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

void GuiTimeLineClip::updateBitmap()
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
        if (mSelected)
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
