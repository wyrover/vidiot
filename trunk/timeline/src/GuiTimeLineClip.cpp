#include "GuiTimeLineClip.h"

#include <wx/dcclient.h>
#include <wx/dcmemory.h>
#include <wx/pen.h>
#include <boost/foreach.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include "GuiTimeLineZoom.h"
#include "UtilLog.h"
#include "AProjectViewNode.h"
#include "VideoFile.h"
#include "VideoClip.h"
#include "EmptyClip.h"
#include "GuiTimeLineTrack.h"

static int sClipBorderSize = 2;

DEFINE_EVENT(CLIP_UPDATE_EVENT, ClipUpdateEvent, GuiTimeLineClipPtr);

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

GuiTimeLineClip::GuiTimeLineClip(GuiTimeLineZoomPtr zoom, 
                                 model::ClipPtr clip)
:   wxEvtHandler()
,   mZoom(zoom)
,   mClip(clip)
,   mThumbnail()
,   mWidth(0)
,   mBitmap()
,   mLink()
,   mTrack()
,   mSelected(false)
,   mBeingDragged(false)
{
}

void GuiTimeLineClip::init(boost::weak_ptr<GuiTimeLineTrack> track, GuiTimeLineClips& allclips)
{
    mTrack = track;
    BOOST_FOREACH( GuiTimeLineClipPtr guiClip, allclips )
    {
        /** /todo time consuming (n*n) */
        if (mClip->getLink() == guiClip->getClip())
        {
            mLink = guiClip;
            break;
        }
    }
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
    mBitmap.Create(mWidth,getTrack()->getBitmap().GetHeight());
    updateThumbnail();
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
    updateBitmap();
}

void GuiTimeLineClip::updateBitmap()
{
    wxMemoryDC dc(mBitmap);
    wxColour* borderColour = const_cast<wxColour*>(wxRED);
    wxBrush* backgroundBrush = const_cast<wxBrush*>(wxCYAN_BRUSH);

    model::EmptyClipPtr emptyclip =  boost::dynamic_pointer_cast<model::EmptyClip>(mClip);

    if (emptyclip)
    {
        
    }
    else
    {
        if (mSelected)
        {
            borderColour = const_cast<wxColour*>(wxBLACK);
            backgroundBrush = new wxBrush(wxColour(80,80,80),wxBRUSHSTYLE_SOLID);//const_cast<wxBrush*>(wxBLACK_BRUSH);
        }
        dc.SetBrush(*backgroundBrush);
        dc.SetPen(wxPen(*borderColour, sClipBorderSize));
        dc.DrawRectangle(0,0,mWidth,mBitmap.GetHeight());
        if (mThumbnail)
        {
            dc.DrawBitmap(*mThumbnail,wxPoint(sClipBorderSize,sClipBorderSize));
        }
    }

    QueueEvent(new ClipUpdateEvent(shared_from_this()));
}

bool GuiTimeLineClip::isSelected() const
{
    return mSelected;
}

void GuiTimeLineClip::setSelected(bool selected)
{
    if (selected != mSelected)
    {
        // This if statement is needed to avoid endless 
        // loops when also selecting the linked clip.
        mSelected = selected;
        updateBitmap();
        GuiTimeLineClipPtr link = getLink();
        if (link)
        {
            link->setSelected(selected);
        }
    }
}

void GuiTimeLineClip::setBeingDragged(bool beingdragged)
{
    mBeingDragged = beingdragged;
    // Event is needed to trigger track redraw (without this clip)
    QueueEvent(new ClipUpdateEvent(shared_from_this()));
}

bool GuiTimeLineClip::isBeingDragged()
{
    return mBeingDragged;
}

model::ClipPtr GuiTimeLineClip::getClip() const
{
    return mClip;
}

GuiTimeLineTrackPtr GuiTimeLineClip::getTrack() const
{
    return mTrack.lock();
}

GuiTimeLineClipPtr GuiTimeLineClip::getLink() const
{
    return mLink.lock();
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION 
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void GuiTimeLineClip::serialize(Archive & ar, const unsigned int version)
{
    ar & mZoom;
    ar & mClip;
    //NOT: ar & mLink; These are restored in 'init()'
}
template void GuiTimeLineClip::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void GuiTimeLineClip::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);


