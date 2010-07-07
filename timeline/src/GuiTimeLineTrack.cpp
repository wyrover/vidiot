#include "GuiTimeLineTrack.h"

#include <wx/dcclient.h>
#include <wx/dcmemory.h>
#include <boost/foreach.hpp>
#include <algorithm>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/list.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/make_shared.hpp>
#include <wx/pen.h>
#include "GuiTimeLineZoom.h"
#include "GuiTimeLineClip.h"
#include "UtilLog.h"
#include "GuiTimeLine.h"
#include "AProjectViewNode.h"
#include "Clip.h"
#include "Track.h"

DEFINE_EVENT(TRACK_UPDATE_EVENT, TrackUpdateEvent, GuiTimeLineTrackPtr);

static int sDefaultTrackHeight = 50;
static int sTrackBorderSize = 1;

class ClipCopyFinder
{
public:
    ClipCopyFinder(model::ClipPtr clip)
        :   mClip(clip) 
    {
    }
    bool operator() (GuiTimeLineClipPtr clip) const
    { 
        return clip->getClip() == mClip; 
    }
private:
    model::ClipPtr mClip;
};

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

GuiTimeLineTrack::GuiTimeLineTrack(GuiTimeLineZoomPtr zoom, model::TrackPtr track)
:   wxEvtHandler()
,   mTrack(track)
,   mZoom(zoom)
,   mBitmap()
,   mTimeLine(0)
,   mHeight(sDefaultTrackHeight)
{
    if (mTrack) // It is set upon creation of a new track, and is '0' in case of recovery
    {
        BOOST_FOREACH( model::ClipPtr clip, mTrack->getClips() )
        {
            mClips.push_back(boost::make_shared<GuiTimeLineClip>(mZoom,clip));
        }
    }
}

void GuiTimeLineTrack::init(GuiTimeLine* timeline, GuiTimeLineClips& allclips)
{
    ASSERT(mZoom); // Must be initialized
    ASSERT(mTrack); // Must be initialized

    mTimeLine = timeline;

    mBitmap.Create(mTimeLine->getWidth(),mHeight);

    BOOST_FOREACH( GuiTimeLineClipPtr clip, mClips )
    {
        clip->init(shared_from_this(), allclips);
    }

    updateBitmap(); // Before binding to clip events to avoid a lot of events

    BOOST_FOREACH( GuiTimeLineClipPtr clip, mClips )
    {
        clip->Bind(CLIP_UPDATE_EVENT,    &GuiTimeLineTrack::OnClipUpdated,       this);
    }
    mTrack->Bind(TRACK_EVENT_ADD_CLIPS,     &GuiTimeLineTrack::OnClipsAdded,    this);
    mTrack->Bind(TRACK_EVENT_REMOVE_CLIPS,  &GuiTimeLineTrack::OnClipsRemoved,  this);
}

GuiTimeLineTrack::~GuiTimeLineTrack()
{
    BOOST_FOREACH( GuiTimeLineClipPtr clip, mClips )
    {
        clip->Unbind(CLIP_UPDATE_EVENT,    &GuiTimeLineTrack::OnClipUpdated,       this);
    }
}

int GuiTimeLineTrack::getClipHeight() const 
{
    return mHeight - 2 * sTrackBorderSize;
}

int GuiTimeLineTrack::getIndex()
{
    return mTimeLine->getIndex(shared_from_this());
}

const wxBitmap& GuiTimeLineTrack::getBitmap()
{
    return mBitmap;
}

GuiTimeLineClips GuiTimeLineTrack::getClips() const
{
    return mClips;
}

boost::tuple<GuiTimeLineClipPtr,int> GuiTimeLineTrack::findClip(int position)
{
    int left = sTrackBorderSize;
    int right = left;
    BOOST_FOREACH( GuiTimeLineClipPtr clip, mClips )
    {
        int width = clip->getBitmap().GetWidth();
        right += width;
        if (position >= left && position <= right)
        {
            return boost::make_tuple(clip,left);
        }
        left += width;
    }
    return boost::make_tuple(GuiTimeLineClipPtr(),0);
}

boost::tuple<int,int> GuiTimeLineTrack::findClipBounds(GuiTimeLineClipPtr findclip)
{
    int left = sTrackBorderSize;
    int right = left;
    BOOST_FOREACH( GuiTimeLineClipPtr clip, mClips )
    {
        int width = clip->getBitmap().GetWidth();
        right += width;
        if (clip == findclip)
        {
            return boost::make_tuple(left,right);
        }
        left += width;
    }
    FATAL("Clip not found.");
    return boost::make_tuple(0,0);
}

//////////////////////////////////////////////////////////////////////////
// MODEL EVENTS
//////////////////////////////////////////////////////////////////////////

void GuiTimeLineTrack::OnClipsAdded( TrackEventAddClips& event )
{
    GuiTimeLineClips newClips;
    BOOST_FOREACH( model::ClipPtr clip, event.getClips() )
    {
        newClips.push_back(boost::make_shared<GuiTimeLineClip>(mZoom,clip));

    }

    GuiTimeLineClips::iterator itPosition = find_if(mClips.begin(), mClips.end(), ClipCopyFinder(event.getPosition()) );
    // NOT: ASSERT(itPosition != mClips.end()); Clips may be added at the end

    mClips.splice(itPosition, newClips);

    updateBitmap();
}

void GuiTimeLineTrack::OnClipsRemoved( TrackEventRemoveClips& event )
{
    GuiTimeLineClips::iterator itBegin = find_if(mClips.begin(), mClips.end(), ClipCopyFinder(event.getClips().front()));
    ASSERT(itBegin != mClips.end());

    GuiTimeLineClips::iterator itEnd = find_if(mClips.begin(), mClips.end(), ClipCopyFinder(event.getClips().back()));
    ASSERT(itEnd != mClips.end());

    ++itEnd; // See http://www.cplusplus.com/reference/stl/list/erase (one but last is removed)
    mClips.erase(itBegin,itEnd);
}

//////////////////////////////////////////////////////////////////////////
// DRAWING EVENTS
//////////////////////////////////////////////////////////////////////////

void GuiTimeLineTrack::OnClipUpdated( ClipUpdateEvent& event )
{
    /** todo only redraw clip */
    updateBitmap();
}

void GuiTimeLineTrack::updateBitmap()
{
    wxMemoryDC dc(mBitmap);
    dc.SetBrush(*wxLIGHT_GREY_BRUSH);
    wxPen blackLinePen(*wxBLACK, sTrackBorderSize);
    dc.SetPen(blackLinePen);
    dc.DrawRectangle(0,0,mBitmap.GetWidth(),mBitmap.GetHeight());

    wxPoint pos(sTrackBorderSize,sTrackBorderSize);
    drawClips(pos, dc);
    QueueEvent(new TrackUpdateEvent(shared_from_this()));
}

void GuiTimeLineTrack::drawClips(wxPoint position, wxMemoryDC& dc, boost::optional<wxMemoryDC&> dcSelectedClipsMask) const
{
    // if dcSelectedClipsMask holds, then we're drawing a 'drag image'. Otherwise, we're drawing the regular track bitmap.
    bool draggedClipsOnly = dcSelectedClipsMask;

    wxPoint pos(position);
    BOOST_FOREACH( GuiTimeLineClipPtr clip, mClips )
    {
        wxBitmap bitmap = clip->getBitmap();

        if (draggedClipsOnly)
        {
            // Drawing the dragged clips
            if (clip->isBeingDragged())
            {
                dc.DrawBitmap(bitmap,pos);
                dcSelectedClipsMask->DrawRectangle(pos,bitmap.GetSize());
            }
        }
        else
        {
            // Regular track drawing
            if (!clip->isBeingDragged())
            {
                dc.DrawBitmap(bitmap,pos);
            }
        }
        pos.x += bitmap.GetWidth();
    }
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION 
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void GuiTimeLineTrack::serialize(Archive & ar, const unsigned int version)
{
    ar & mTrack;
    ar & mClips;
    ar & mZoom;
    ar & mHeight;
}
template void GuiTimeLineTrack::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void GuiTimeLineTrack::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);


