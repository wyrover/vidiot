#include "GuiTimeLineClip.h"

#include <wx/dcclient.h>
#include <wx/dcmemory.h>
#include <wx/pen.h>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include "GuiTimeLineZoom.h"
#include "UtilLog.h"
#include "AProjectViewNode.h"
#include "VideoFile.h"
#include "VideoClip.h"
#include "GuiTimeLineTrack.h"

static int sClipBorderSize = 2;

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

GuiTimeLineClip::GuiTimeLineClip(GuiTimeLineZoomPtr zoom, 
                                 model::ClipPtr clip)
:   wxEvtHandler()
,   mZoom(zoom)
,   mClip(clip) /** /todo how to determine linked clip */
,   mThumbnail()
,   mWidth(0)
,   mBitmap()
,   mTrack(0)
,   mSelected(false)
,   mBeingDragged(false)
{
}

void GuiTimeLineClip::init(GuiTimeLineTrack* track)
{
    mTrack = track;
    updateSize(); // Also creates bitmap
}

GuiTimeLineClip::~GuiTimeLineClip()
{
}

const wxBitmap& GuiTimeLineClip::getBitmap()
{
    return mBitmap;
}

void GuiTimeLineClip::updateSize()
{
    mWidth = mZoom->ptsToPixels(mClip->getNumberOfFrames());
    mBitmap.Create(mWidth,mTrack->getBitmap().GetHeight());
    updateThumbnail();
    updateBitmap();
}

void GuiTimeLineClip::updateThumbnail()
{
    model::VideoClipPtr videoclip = boost::dynamic_pointer_cast<model::VideoClip>(mClip);
    if (videoclip)
    {
        VideoFramePtr videoFrame = videoclip->getNextVideo(mWidth, mBitmap.GetHeight() - 2 * sClipBorderSize, false);
        mThumbnail.reset(new wxBitmap(wxImage(videoFrame->getWidth(), videoFrame->getHeight(), videoFrame->getData()[0], true)));
        mClip->moveTo(0);
    }
}

void GuiTimeLineClip::updateBitmap()
{
    wxMemoryDC dc(mBitmap);
    wxColour* borderColour = const_cast<wxColour*>(wxRED);
    wxBrush* backgroundBrush = const_cast<wxBrush*>(wxCYAN_BRUSH);
    if (mSelected)
    {
        borderColour = const_cast<wxColour*>(wxBLACK);
        backgroundBrush = const_cast<wxBrush*>(wxBLACK_BRUSH);
    }
    dc.SetBrush(*backgroundBrush);
    dc.SetPen(wxPen(*borderColour, sClipBorderSize));
    dc.DrawRectangle(0,0,mWidth,mBitmap.GetHeight());
    if (mThumbnail)
    {
        dc.DrawBitmap(*mThumbnail,wxPoint(sClipBorderSize,sClipBorderSize));
    }
    QueueEvent(new ClipUpdateEvent(shared_from_this()));
}

bool GuiTimeLineClip::isSelected() const
{
    return mSelected;
}

void GuiTimeLineClip::setSelected(bool selected)
{
    mSelected = selected;
    QueueEvent(new ClipSelectionEvent(shared_from_this(), mSelected));
    updateBitmap();
}

void GuiTimeLineClip::setBeingDragged(bool beingdragged)
{
    mBeingDragged = beingdragged;
}

bool GuiTimeLineClip::isBeingDragged()
{
    return mBeingDragged;
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION 
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void GuiTimeLineClip::serialize(Archive & ar, const unsigned int version)
{
    ar & mZoom;
    ar & mClip;
}
template void GuiTimeLineClip::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void GuiTimeLineClip::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);


