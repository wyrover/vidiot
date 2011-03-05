#include "Divider.h"
#include <wx/dc.h>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
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

Divider::Divider(Timeline* timeline)
:   Part(timeline)
,   mPosition(0)
{
    VAR_DEBUG(this);

    getSequence()->Bind(model::EVENT_ADD_VIDEO_TRACK, &Divider::onVideoTracksAdded, this);
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
    resetPosition();
    event.Skip();
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

int Divider::getPosition() const
{
    return mPosition;
}

void Divider::setPosition(int position)
{
    int minimum = Layout::sVideoPosition + getSequenceView().getVideo().getBitmap().GetHeight();
    if (position < minimum)
    {
        position = minimum;
    }
    mPosition = position;
    getSequenceView().invalidateBitmap();
}

void Divider::resetPosition()
{
    setPosition(mPosition);
}

void Divider::getPositionInfo(wxPoint position, PointerPositionInfo& info ) const
{
     info.onAudioVideoDivider =
         position.y >= getPosition() && 
         position.y <= getAudioPosition();
}


int Divider::getAudioPosition() const
{
    return getPosition() + Layout::sAudioVideoDividerHeight;
}

int Divider::getVideoPosition() const
{
    return getPosition() - getSequenceView().getVideo().getBitmap().GetHeight();
}



//////////////////////////////////////////////////////////////////////////
// DRAWING
//////////////////////////////////////////////////////////////////////////

void Divider::draw(wxDC& dc) const
{
    dc.SetBrush(Layout::sAudioVideoDividerBrush);
    dc.SetPen(Layout::sAudioVideoDividerPen);
    dc.DrawRectangle(0,mPosition,dc.GetSize().GetWidth(),Layout::sAudioVideoDividerHeight);
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void Divider::serialize(Archive & ar, const unsigned int version)
{
    ar & mPosition;
}

template void Divider::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void Divider::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);

}} // namespace
