#ifndef TRIM_EVENT_H
#define TRIM_EVENT_H

#include "UtilEnum.h"
#include "UtilEvent.h"
#include "UtilInt.h"

namespace gui { namespace timeline {

DECLAREENUM(OperationState, \
    OperationStateStart, \
    OperationStateUpdate, \
    OperationStateStop);

class TrimEvent
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    explicit TrimEvent(
        OperationState state,
        model::IClipPtr clip = model::IClipPtr(),
        model::IClipPtr link = model::IClipPtr(),
        model::IClipPtr cliptrimmed = model::IClipPtr(),
        model::IClipPtr linktrimmed = model::IClipPtr());

    TrimEvent(const TrimEvent& other);

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    OperationState getState() const;
    model::IClipPtr getClip() const;
    model::IClipPtr getClipTrimmed() const;
    model::IClipPtr getLink() const;
    model::IClipPtr getLinkTrimmed() const;

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    OperationState mState;
    model::IClipPtr mClip;
    model::IClipPtr mLink;
    model::IClipPtr mClipTrimmed;
    model::IClipPtr mLinkTrimmed;

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<<( std::ostream& os, const TrimEvent& obj );
};

DECLARE_EVENT(EVENT_TRIM_UPDATE, EventTrimUpdate, TrimEvent);

}} // namespace

#endif // TRIM_EVENT_H