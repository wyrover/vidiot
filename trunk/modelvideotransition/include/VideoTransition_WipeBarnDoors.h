// Copyright 2013-2016 Eric Raijmakers.
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

#pragma once

#include "VideoTransitionOpacity.h"

namespace model { namespace video { namespace transition {

class WipeBarnDoors
    :   public VideoTransitionOpacity
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    WipeBarnDoors() = default;

    WipeBarnDoors* clone() const override;

    virtual ~WipeBarnDoors() = default;

    //////////////////////////////////////////////////////////////////////////
    // TRANSITION
    //////////////////////////////////////////////////////////////////////////

    bool supports(TransitionType type) const override;

    std::vector<std::tuple<wxString, wxString, TransitionParameterPtr>> getAvailableParameters() const override;

    wxString getDescription(TransitionType type) const override;

    //////////////////////////////////////////////////////////////////////////
    // VIDEOTRANSITIONOPACITY
    //////////////////////////////////////////////////////////////////////////

    std::function<float(int, int)> getLeftMethod(const wxImagePtr& image, const float& factor) const override;

    std::function<float (int,int)> getRightMethod(const wxImagePtr& image, const float& factor) const override;

protected:

    //////////////////////////////////////////////////////////////////////////
    // COPY CONSTRUCTOR
    //////////////////////////////////////////////////////////////////////////
public:
    /// Copy constructor. Use make_cloned for making deep copies of objects.
    /// \see make_cloned
    WipeBarnDoors(const WipeBarnDoors& other) = default;

private:

    //////////////////////////////////////////////////////////////////////////
    // SERIALIZATION
    //////////////////////////////////////////////////////////////////////////

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);
};

}}} // namespace

BOOST_CLASS_VERSION(model::video::transition::WipeBarnDoors, 1)
BOOST_CLASS_EXPORT_KEY(model::video::transition::WipeBarnDoors)
