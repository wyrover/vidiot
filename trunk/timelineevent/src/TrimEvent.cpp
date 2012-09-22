#include "TrimEvent.h"
#include "Config.h"

namespace gui { namespace timeline {

IMPLEMENTENUM(OperationState);
DEFINE_EVENT(EVENT_TRIM_UPDATE, EventTrimUpdate, TrimEvent);

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

TrimEvent::TrimEvent(
        OperationState state,
        model::IClipPtr clip,
        model::IClipPtr link,
        model::IClipPtr cliptrimmed,
        model::IClipPtr linktrimmed)
    :   mState(state)
    ,   mClip(clip)
    ,   mLink(link)
    ,   mClipTrimmed(cliptrimmed)
    ,   mLinkTrimmed(linktrimmed)
{
}

TrimEvent::TrimEvent(const TrimEvent& other)
    :   mState(other.mState)
    ,   mClip(other.mClip)
    ,   mLink(other.mLink)
    ,   mClipTrimmed(other.mClipTrimmed)
    ,   mLinkTrimmed(other.mLinkTrimmed)
{
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

OperationState TrimEvent::getState() const
{
    return mState;
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
    os << obj.mState << '|' << obj.mClip<< '|' << obj.mClipTrimmed << '|' << obj.mLink << '|' << obj.mLinkTrimmed;
    return os;
}

}} // namespace