#include "Divider.h"
#include <wx/dc.h>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include "UtilLog.h"
#include "Constants.h"
#include "VideoView.h"
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
    LOG_INFO;
}

Divider::~Divider()
{
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
    int minimum = Constants::sVideoPosition + getTimeline().getVideo().getBitmap().GetHeight();
    if (position < minimum)
    {
        position = minimum;
    }
    mPosition = position;
    getTimeline().invalidateBitmap();
}

void Divider::getPositionInfo(wxPoint position, PointerPositionInfo& info ) const
{
     info.onAudioVideoDivider =
         position.y >= getPosition() && 
         position.y <= getAudioPosition();
}


int Divider::getAudioPosition() const
{
    return getPosition() + Constants::sAudioVideoDividerHeight;
}

int Divider::getVideoPosition() const
{
    return getPosition() - getTimeline().getVideo().getBitmap().GetHeight();
}



//////////////////////////////////////////////////////////////////////////
// DRAWING
//////////////////////////////////////////////////////////////////////////

void Divider::draw(wxDC& dc) const
{
    dc.SetBrush(Constants::sAudioVideoDividerBrush);
    dc.SetPen(Constants::sAudioVideoDividerPen);
    dc.DrawRectangle(0,mPosition,dc.GetSize().GetWidth(),Constants::sAudioVideoDividerHeight);
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
