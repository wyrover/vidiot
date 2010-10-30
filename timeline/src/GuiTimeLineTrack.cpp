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
#include "Constants.h"
#include "GuiTimeLineZoom.h"
#include "GuiTimeLineClip.h"
#include "UtilLog.h"
#include "GuiTimeLine.h"
#include "AProjectViewNode.h"
#include "UtilLogStl.h"
#include "Clip.h"

namespace gui { namespace timeline {

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

GuiTimeLineTrack::GuiTimeLineTrack(const GuiTimeLineZoom& zoom, 
                                   ViewMap& viewMap, 
                                   model::TrackPtr track)
:   wxEvtHandler()
,   mTrack(track)
,   mZoom(zoom)
,   mViewMap(viewMap)
,   mBitmap()
,   mTimeLine(0)
,   mHeight(sDefaultTrackHeight)
{
    ASSERT(mTrack); // Must be initialized

}

void GuiTimeLineTrack::init(GuiTimeLine* timeline)
{
    mViewMap.add(mTrack,shared_from_this());

    mTimeLine = timeline;

    mBitmap.Create(mTimeLine->getWidth(),mHeight);

    BOOST_FOREACH( model::ClipPtr clip, mTrack->getClips() )
    {
        GuiTimeLineClipPtr gClip( new GuiTimeLineClip(mZoom, mViewMap, clip));
        mClips.push_back(gClip);
        mViewMap.add(clip,gClip);
        gClip->init();
    }

    updateBitmap(); // Before binding to clip events to avoid a lot of events

    BOOST_FOREACH( GuiTimeLineClipPtr clip, mClips )
    {
        clip->Bind(CLIP_UPDATE_EVENT,    &GuiTimeLineTrack::OnClipUpdated,       this);
    }
    mTrack->Bind(model::EVENT_ADD_CLIPS,     &GuiTimeLineTrack::OnClipsAdded,    this);
    mTrack->Bind(model::EVENT_REMOVE_CLIPS,  &GuiTimeLineTrack::OnClipsRemoved,  this);

    VAR_DEBUG(mClips);
}

GuiTimeLineTrack::~GuiTimeLineTrack()
{
    BOOST_FOREACH( GuiTimeLineClipPtr clip, mClips )
    {
        clip->Unbind(CLIP_UPDATE_EVENT,    &GuiTimeLineTrack::OnClipUpdated,       this);
    }
    mViewMap.remove(shared_from_this());
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

GuiTimeLineClipWithOffset GuiTimeLineTrack::findClip(int position)
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

model::TrackPtr GuiTimeLineTrack::getTrack() const
{
    return mTrack;
}

//////////////////////////////////////////////////////////////////////////
// MODEL EVENTS
//////////////////////////////////////////////////////////////////////////

void GuiTimeLineTrack::OnClipsAdded( model::EventAddClips& event )
{
    GuiTimeLineClips newClips;
    BOOST_FOREACH( model::ClipPtr clip, event.getValue().addClips )
    {
        GuiTimeLineClipPtr gclip(new GuiTimeLineClip(mZoom,mViewMap,clip));
        mViewMap.add(clip,gclip);
        gclip->init();
        gclip->Bind(CLIP_UPDATE_EVENT,    &GuiTimeLineTrack::OnClipUpdated,       this);
        newClips.push_back(gclip);
    }

    GuiTimeLineClips::iterator itPosition = find_if(mClips.begin(), mClips.end(), ClipCopyFinder(event.getValue().addPosition) );
    // NOT: ASSERT(itPosition != mClips.end()); Clips may be added at the end

    mClips.splice(itPosition, newClips);

    VAR_DEBUG(mClips);

    updateBitmap();
}

void GuiTimeLineTrack::OnClipsRemoved( model::EventRemoveClips& event )
{
    BOOST_FOREACH( model::ClipPtr clip, event.getValue().removeClips )
    {
        mViewMap.remove(clip);
    }

    GuiTimeLineClips::iterator itBegin = find_if(mClips.begin(), mClips.end(), ClipCopyFinder(event.getValue().removeClips.front()));
    ASSERT(itBegin != mClips.end());

    GuiTimeLineClips::iterator itEnd = find_if(mClips.begin(), mClips.end(), ClipCopyFinder(event.getValue().removeClips.back()));
    ASSERT(itEnd != mClips.end());

    ++itEnd; // See http://www.cplusplus.com/reference/stl/list/erase (one but last is removed)
    mClips.erase(itBegin,itEnd);

    VAR_DEBUG(mClips);

    updateBitmap();
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
    dc.SetBrush(Constants::sBackgroundBrush);
    dc.SetPen(Constants::sBackgroundPen);
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

}} // namespace

