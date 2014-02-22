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

#ifndef ZOOM_H
#define ZOOM_H

#include "Part.h"
#include "UtilEvent.h"
#include "UtilInt.h"

namespace gui { namespace timeline {

class Zoom
    :   public Part
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION METHODS
    //////////////////////////////////////////////////////////////////////////

    Zoom(Timeline* timeline);
    virtual ~Zoom();

    //////////////////////////////////////////////////////////////////////////
    // ZOOM
    //////////////////////////////////////////////////////////////////////////

    // Return current zoom level
    rational getCurrent() const;

    /// Change the current zoom level
    /// \param steps number of steps to change (>0 zoom in, <0 zoom out)
    void change(int steps);

    //////////////////////////////////////////////////////////////////////////
    // ZOOM RESULT
    //////////////////////////////////////////////////////////////////////////

    int timeToPixels(int time) const;       ///< @param time time duration in milliseconds
    int pixelsToTime(int pixels) const;     ///< @return time duration in milliseconds
    pts pixelsToPts(int pixels) const;
    int ptsToPixels(pts position) const;

private:

    rational mZoom; ///< Number of pixels per frame

    //////////////////////////////////////////////////////////////////////////
    // SERIALIZATION
    //////////////////////////////////////////////////////////////////////////

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);
};

}} // namespace

// Workaround needed to prevent compile-time errors (mpl_assertion_in_line...) with gcc
//#include  <boost/preprocessor/slot/counter.hpp>
//#include BOOST____PP_UPDATE_COUNTER()
//#line BOOST_____PP_COUNTER
BOOST_CLASS_VERSION(gui::timeline::Zoom, 1)

#endif // ZOOM_H