#include "TrimEvent.h"

namespace gui { namespace timeline {

DEFINE_EVENT(EVENT_TRIM_UPDATE, EventTrimUpdate, TrimEvent);

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

TrimEvent::TrimEvent(
        bool active,
        model::IClipPtr clip,
        model::IClipPtr link,
        model::IClipPtr cliptrimmed,
        model::IClipPtr linktrimmed)
    :   mActive(active)
    ,   mClip(clip)
    ,   mLink(link)
    ,   mClipTrimmed(cliptrimmed)
    ,   mLinkTrimmed(linktrimmed)
{
}

TrimEvent::TrimEvent(const TrimEvent& other)
    :   mActive(other.mActive)
    ,   mClip(other.mClip)
    ,   mLink(other.mLink)
    ,   mClipTrimmed(other.mClipTrimmed)
    ,   mLinkTrimmed(other.mLinkTrimmed)
{
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

bool TrimEvent::getActive() const
{
    return mActive;
}

model::IClipPtr TrimEvent::getClip() const
{
    return mClip;
}

model::IClipPtr TrimEvent::getClipTrimmed() const
{
    return mClipTrimmed;
}

model::IClipPtr TrimEvent::getLink() const
{
    return mLink;
}

model::IClipPtr TrimEvent::getLinkTrimmed() const
{
    return mLinkTrimmed;
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<( std::ostream& os, const TrimEvent& obj )
{
    os << obj.mActive << '|' << obj.mClip<< '|' << obj.mClipTrimmed << '|' << obj.mLink << '|' << obj.mLinkTrimmed;
    return os;
}

}} // namespace