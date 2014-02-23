// Copyright 2013,2014 Eric Raijmakers.
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

#ifndef SCROLL_H
#define SCROLL_H

#include "Part.h"
#include "UtilEvent.h"
#include "UtilInt.h"

namespace gui { namespace timeline {

/// Virtual position:  position in the virtual area. That is, the entire area
///                    that is accessible when using the scrollbars.
/// Physical position: position in coordinates of the viewable area.
class Scrolling
    :   public Part
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION METHODS
    //////////////////////////////////////////////////////////////////////////

    Scrolling(Timeline* timeline);
    virtual ~Scrolling();

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    /// Return the current scrolling offset
    wxPoint getOffset() const;

    /// Align the scroll position in such a way that the position 'sequencePts'
    /// is directly under the physical position.
    /// \param position position in the sequence
    /// \param unscrolledPixel physical position to align to
    /// \return if the scroll was moved to the beginning, and that still wasn't enough for the alignment, the difference (the amount that remains to be scrolled) is returned
    pixel align(pts position, pixel physicalPosition);

    /// \param position position in the sequence
    /// \return physical position on the scroll window canvas
    /// \note this conversion includes the required computations for taking zooming into account
    pixel ptsToPixel(pts position) const;

    /// \return physical position
    /// \param position virtual position
    wxPoint getVirtualPosition(wxPoint position) const;

    /// Store the current center pts position. This position is used when the
    /// zoom is updated, to ensure that approximately the same area is viewed
    /// after the zoom.
    void storeCenterPts();

    /// \return the pts shown at the center of the timeline
    /// \note if the scrollbar is at its left most position, then '0' is returned.
    pts getCenterPts() const;

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    void alignCenterPts();

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    pts mCenterPts;

    //////////////////////////////////////////////////////////////////////////
    // SERIALIZATION
    //////////////////////////////////////////////////////////////////////////

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);
};

}} // namespace

#endif
