// Copyright 2013 Eric Raijmakers.
//
// This file is part of Vidiot.
//
// Vidiot is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Vidiot is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Vidiot. If not, see <http://www.gnu.org/licenses/>.

#ifndef POSITION_INFO_H
#define POSITION_INFO_H

#include "UtilEnum.h"

namespace gui { namespace timeline {

    /**
    *
    *                                K->   <--N-->   <-Q
    *        ------------------------|-----------------|------------------------
    *        |                       |                 |                       |   ^
    *        |                       |                 |                       |   |
    *        |                       |-----------------|                       |   y
    *        |                                |                                |   |
    *        |                                |                                |   v
    *        |                                |                                |
    *        -------------------------------------------------------------------
    *
    *        A->    <----B---->      L->   <-M O->   <-P     <----B---->     <-C
    *
    *
    * A: ClipBegin
    * B: ClipInterior
    * C: ClipEnd
    *
    * K: TransitionBegin                (y <= transition height)
    * L: TransitionLeftClipInterior     (y >  transition height)
    * M: TransitionLeftClipEnd          (y >  transition height)
    * N: TransitionInterior             (y <= transition height)
    * O: TransitionRightClipBegin       (y >  transition height)
    * P: TransitionRightClipInterior    (y >  transition height)
    * Q: TransitionEnd                  (y <= transition height)
    */
DECLAREENUM(MouseOnClipPosition, \
            ClipBegin, \
            ClipInterior, \
            ClipEnd, \
            TransitionBegin, \
            TransitionLeftClipInterior, \
            TransitionLeftClipEnd, \
            TransitionInterior, \
            TransitionRightClipBegin, \
            TransitionRightClipInterior, \
            TransitionEnd);

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

    model::TrackPtr track;  ///< Current track under the mouse pointer. 0 if none.
    int trackPosition;      ///< Y position of current track. 0 if no current track.
    bool onTrackDivider;    ///< True if pointer is ON the track divider for this track.

    //////////////////////////////////////////////////////////////////////////
    // CLIP
    //////////////////////////////////////////////////////////////////////////

    model::IClipPtr clip;   ///< Current clip under the mouse pointer. 0 if none.
    MouseOnClipPosition logicalclipposition;
};

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<( std::ostream& os, const PointerPositionInfo& obj );

}} // namespace

#endif // POSITION_INFO_H