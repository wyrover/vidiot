#ifndef TRIM_EVENT_H
#define TRIM_EVENT_H

#include "UtilEvent.h"
#include "UtilInt.h"

namespace gui { namespace timeline {

class TrimEvent
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    explicit TrimEvent(
        bool active = false,
        model::IClipPtr clip = model::IClipPtr(),
        model::IClipPtr link = model::IClipPtr(),
        model::IClipPtr cliptrimmed = model::IClipPtr(),
        model::IClipPtr linktrimmed = model::IClipPtr());

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    bool getActive() const;
    model::IClipPtr getClip() const;
    model::IClipPtr getClipTrimmed() const;
    model::IClipPtr getLink() const;
    model::IClipPtr getLinkTrimmed() const;

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    bool mActive;
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