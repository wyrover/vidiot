#ifndef POSITION_INFO_H
#define POSITION_INFO_H

#include <boost/shared_ptr.hpp>
#include "UtilEnum.h"

namespace model {
    class Track;
    typedef boost::shared_ptr<Track> TrackPtr;
    class IClip;
    typedef boost::shared_ptr<IClip> IClipPtr;
}

namespace gui { namespace timeline {

DECLAREENUM(MouseOnClipPosition, \
            ClipBegin, \
            ClipInterior, \
            ClipEnd);

/** @todo split into track info and 'withintrack' info */
struct PointerPositionInfo
{
    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    PointerPositionInfo();
    ~PointerPositionInfo();

    //////////////////////////////////////////////////////////////////////////
    // NON-SEQUENCE OBJECTS
    //////////////////////////////////////////////////////////////////////////

    bool onAudioVideoDivider;

    //////////////////////////////////////////////////////////////////////////
    // TRACK
    //////////////////////////////////////////////////////////////////////////

    /** Current track under the mouse pointer. 0 if none. */
    model::TrackPtr track;

    /** Y position of current track. 0 if no current track. */
    int trackPosition;

    /** True if pointer is ON the track divider for this track. */
    bool onTrackDivider;

    //////////////////////////////////////////////////////////////////////////
    // CLIP
    //////////////////////////////////////////////////////////////////////////

    /** Current clip under the mouse pointer. 0 if none. */
    model::IClipPtr clip;

    MouseOnClipPosition logicalclipposition;
};

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<( std::ostream& os, const PointerPositionInfo& obj );

}} // namespace

#endif // POSITION_INFO_H