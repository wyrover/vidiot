#include "GuiTimeLineTrack.h"

#include <wx/dcclient.h>
#include <wx/dcmemory.h>
#include <boost/foreach.hpp>
#include <algorithm>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/list.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <wx/pen.h>
#include "Constants.h"
#include "Zoom.h"
#include "GuiTimeLineClip.h"
#include "UtilLog.h"
#include "GuiTimeLine.h"
#include "AProjectViewNode.h"
#include "UtilLogStl.h"
#include "Clip.h"
#include "GuiMain.h"

namespace gui { namespace timeline {

DEFINE_EVENT(TRACK_UPDATE_EVENT, TrackUpdateEvent, GuiTimeLineTrack*);

static int sTrackBorderSize = 1;

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

GuiTimeLineTrack::GuiTimeLineTrack(model::TrackPtr track)
:   wxWindow()
,   mTrack(track)
,   mBitmap()
,   mRedrawOnIdle(false)
{
    ASSERT(mTrack); // Must be initialized
}

void GuiTimeLineTrack::init()
{
    Create(&getTimeline(), wxID_ANY);

    getViewMap().registerView(mTrack,this);
    mBitmap.Create(getTimeline().getWidth(),mTrack->getHeight());

    model::MoveParameter m;
    m.addClips = mTrack->getClips();
    OnClipsAdded(model::EventAddClips(m));

    mRedrawOnIdle = true;

    Bind(wxEVT_IDLE, &GuiTimeLineTrack::OnIdle, this);

    mTrack->Bind(model::EVENT_ADD_CLIPS,     &GuiTimeLineTrack::OnClipsAdded,    this);
    mTrack->Bind(model::EVENT_REMOVE_CLIPS,  &GuiTimeLineTrack::OnClipsRemoved,  this);

    Hide(); // Otherwise a default widget is painted in the topleft corner of the timeline
}

GuiTimeLineTrack::~GuiTimeLineTrack()
{
    getViewMap().unregisterView(mTrack);
}

int GuiTimeLineTrack::getClipHeight() const
{
    return mTrack->getHeight() - 2 * sTrackBorderSize;
}

const wxBitmap& GuiTimeLineTrack::getBitmap()
{
    return mBitmap;
}

model::TrackPtr GuiTimeLineTrack::getTrack() const
{
    return mTrack;
}

//////////////////////////////////////////////////////////////////////////
// GUI EVENTS
//////////////////////////////////////////////////////////////////////////

void GuiTimeLineTrack::OnIdle(wxIdleEvent& event)
{
    if (mRedrawOnIdle)
    {
        // This is done to avoid using intermediary states for iterating though
        // the model. For instance, when replacing clips with other clips, first
        // a unregisterView event and then a registerView event is received. However, while 
        // receiving the unregisterView event, the actual adding may already have been
        // done. Then the view for the added clips has not yet been initialized.
        updateBitmap();
        mRedrawOnIdle = false;
    }
    event.Skip();
}

//////////////////////////////////////////////////////////////////////////
// MODEL EVENTS
//////////////////////////////////////////////////////////////////////////

void GuiTimeLineTrack::OnClipsAdded( model::EventAddClips& event )
{
    BOOST_FOREACH( model::ClipPtr clip, event.getValue().addClips )
    {
        GuiTimeLineClip* p = new GuiTimeLineClip(this,clip);
        p->initTimeline(&getTimeline());
        p->Bind(CLIP_UPDATE_EVENT, &GuiTimeLineTrack::OnClipUpdated, this); // After init to avoid initial events (since updateBitmap below redraws the entire bitmap)
    }
    mRedrawOnIdle = true;
}

void GuiTimeLineTrack::OnClipsRemoved( model::EventRemoveClips& event )
{
    BOOST_FOREACH( model::ClipPtr clip, event.getValue().removeClips )
    {
        getViewMap().getView(clip)->Unbind(CLIP_UPDATE_EVENT, &GuiTimeLineTrack::OnClipUpdated, this);
        getViewMap().getView(clip)->Destroy();
    }
    mRedrawOnIdle = true;
}

//////////////////////////////////////////////////////////////////////////
// DRAWING EVENTS
//////////////////////////////////////////////////////////////////////////

void GuiTimeLineTrack::OnClipUpdated( ClipUpdateEvent& event )
{
    /** todo only redraw clip */
    mRedrawOnIdle = true;
}

void GuiTimeLineTrack::updateBitmap()
{
    wxMemoryDC dc(mBitmap);
    dc.SetBrush(Constants::sBackgroundBrush);
    dc.SetPen(Constants::sBackgroundPen);
    dc.DrawRectangle(0,0,mBitmap.GetWidth(),mBitmap.GetHeight());

    wxPoint pos(sTrackBorderSize,sTrackBorderSize);
    drawClips(pos, dc);
    QueueEvent(new TrackUpdateEvent(this));
}

void GuiTimeLineTrack::drawClips(wxPoint position, wxMemoryDC& dc, boost::optional<wxMemoryDC&> dcSelectedClipsMask)
{
    // if dcSelectedClipsMask holds, then we're drawing a 'drag image'. Otherwise, we're drawing the regular track bitmap.
    bool draggedClipsOnly = dcSelectedClipsMask;

    wxPoint pos(position);
    BOOST_FOREACH( model::ClipPtr modelclip, mTrack->getClips() )
    {
        GuiTimeLineClip* clip = getViewMap().getView(modelclip);
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

