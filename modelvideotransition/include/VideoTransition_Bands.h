// Copyright 2013-2015 Eric Raijmakers.
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

#ifndef TRANSITION_BANDS_H
#define TRANSITION_BANDS_H

#include "VideoTransitionOpacity.h"

namespace model { namespace video { namespace transition {

class Bands
    :   public VideoTransitionOpacity
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    Bands();

    virtual Bands* clone() const;

    virtual ~Bands();

    //////////////////////////////////////////////////////////////////////////
    // TRANSITION
    //////////////////////////////////////////////////////////////////////////

    wxString getDescription(TransitionType type) const override;

    //////////////////////////////////////////////////////////////////////////
    // VIDEOTRANSITIONOPACITY
    //////////////////////////////////////////////////////////////////////////

    void handleFullyOpaqueImage(const wxImagePtr& image, const boost::function<float (int, int)>& f) const override;
    void handleImageWithAlpha(const wxImagePtr& image, const boost::function<float (int, int)>& f) const override;

    boost::function<float (int,int)> getLeftMethod(const wxImagePtr& image, const float& factor) const override;
    boost::function<float (int,int)> getRightMethod(const wxImagePtr& image, const float& factor) const override;

protected:

    //////////////////////////////////////////////////////////////////////////
    // COPY CONSTRUCTOR
    //////////////////////////////////////////////////////////////////////////

    /// Copy constructor. Use make_cloned for making deep copies of objects.
    /// \see make_cloned
    Bands(const Bands& other);

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    int mBands;

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<<(std::ostream& os, const Bands& obj);

    //////////////////////////////////////////////////////////////////////////
    // SERIALIZATION
    //////////////////////////////////////////////////////////////////////////

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);
};

}}} // namespace

BOOST_CLASS_VERSION(model::video::transition::Bands, 1)
BOOST_CLASS_EXPORT_KEY(model::video::transition::Bands)

#endif