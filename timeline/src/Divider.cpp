#include "Divider.h"
#include <wx/dcmemory.h>
#include "UtilLog.h"
#include "Layout.h"
#include "VideoView.h"
#include "Sequence.h"
#include "SequenceView.h"
#include "Timeline.h"
#include "PositionInfo.h"

namespace gui { namespace timeline {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

Divider::Divider(View* parent)
:   View(parent)
{
    VAR_DEBUG(this);
}

Divider::~Divider()
{
    VAR_DEBUG(this);

    getSequence()->Unbind(model::EVENT_ADD_VIDEO_TRACK, &Divider::onVideoTracksAdded, this);
}

//////////////////////////////////////////////////////////////////////////
// MODEL EVENTS
//////////////////////////////////////////////////////////////////////////

void Divider::onVideoTracksAdded( model::EventAddVideoTracks& event )
{
    resetDividerPosition();
    event.Skip();
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

pixel Divider::requiredWidth() const
{
    return getSequenceView().requiredWidth();
}

pixel Divider::requiredHeight() const
{
    return Layout::sAudioVideoDividerHeight;
}

void Divider::setPosition(int position)
{
    int minimum = Layout::sVideoPosition + getSequenceView().getVideo().requiredHeight();
    if (position < minimum)
    {
        position = minimum;
    }
    getSequence()->setDividerPosition(position);
    invalidateBitmap();
    getTimeline().Update();
    // todo ensure updates getSequenceView().invalidateBitmap();
}

void Divider::resetDividerPosition()
{
    setPosition(getSequence()->getDividerPosition());
}

void Divider::getPositionInfo(wxPoint position, PointerPositionInfo& info ) const
{
     info.onAudioVideoDivider =
         position.y >= getSequence()->getDividerPosition() && 
         position.y <= getAudioPosition();
}

int Divider::getAudioPosition() const
{
    return getSequence()->getDividerPosition() + Layout::sAudioVideoDividerHeight;
}

int Divider::getVideoPosition() const
{
    return getSequence()->getDividerPosition() - getSequenceView().getVideo().getBitmap().GetHeight();
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void Divider::draw(wxBitmap& bitmap) const
{
    wxMemoryDC dc(bitmap);
    dc.SetBrush(Layout::sAudioVideoDividerBrush);
    dc.SetPen(Layout::sAudioVideoDividerPen);
    dc.DrawRectangle(wxPoint(0,0),bitmap.GetSize());
}

}} // namespace
