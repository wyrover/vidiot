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

#include "VideoTransition_WipeBarnDoors.h"

#include "TransitionParameterDirection2.h"
#include "TransitionParameterBool.h"

namespace model { namespace video { namespace transition {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

WipeBarnDoors* WipeBarnDoors::clone() const
{
    return new WipeBarnDoors(static_cast<const WipeBarnDoors&>(*this));
}

//////////////////////////////////////////////////////////////////////////
// TRANSITION
//////////////////////////////////////////////////////////////////////////

bool WipeBarnDoors::supports(TransitionType type) const
{
    return true;
}

ParameterAttributes WipeBarnDoors::getAvailableParameters() const
{
    return
    {
        { TransitionParameterDirection2::sParameterDirection2, _("Direction"), _("Select between a horizontal or a vertical effect"), "direction-blue.png", boost::make_shared<TransitionParameterDirection2>(Direction2Horizontal) },
        { TransitionParameterBool::sParameterInversed, _("Inversed"), _("Select between making the second clip visible from the inside (normal) or from the outside (inversed)"), "arrow-return-180.png", boost::make_shared<TransitionParameterBool>(false) },
    };
}

wxString WipeBarnDoors::getDescription(TransitionType type) const
{
    return _("Barn Doors");
}

//////////////////////////////////////////////////////////////////////////
// VIDEOTRANSITIONOPACITY
//////////////////////////////////////////////////////////////////////////

std::function<float (int,int)> WipeBarnDoors::getRightMethod(const wxImagePtr& image, const float& factor) const
{
    Direction2 direction{ getParameter<TransitionParameterDirection2>(TransitionParameterDirection2::sParameterDirection2)->getValue() };
    bool inversed{ getParameter<TransitionParameterBool>(TransitionParameterBool::sParameterInversed)->getValue() };

    int w{ image->GetWidth() };
    int h{ image->GetHeight() };
    float f{ inversed ? 1.0f - factor : factor };
    float inside{ inversed ? 0.0f : 1.0f };
    float outside{ 1.0f - inside };

    switch (direction)
    {
        case Direction2Horizontal:
        {
            int length{ w / 2 };
            int boundary{ narrow_cast<int>(std::floor(f * length)) };
            int left{ (w / 2) - boundary };
            int right{ (w / 2) + boundary };
            return [left, right, inside, outside](int x, int y) -> float
            {
                return (x < left || x >= right) ? outside : inside;
            };
        }
        case Direction2Vertical:
        {
            int length{ h / 2 };
            int boundary{ narrow_cast<int>(std::floor(f * length)) };
            int top{ (h / 2) - boundary };
            int bottom{ (h / 2) + boundary };
            return [top, bottom, inside, outside](int x, int y) -> float
            {
                return (y < top || y >= bottom) ? outside : inside;
            };
        }
        default: { FATAL("Wrong direction"); break; }
    }
    return nullptr;
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void WipeBarnDoors::serialize(Archive & ar, const unsigned int version)
{
    try
    {
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(VideoTransition);
    }
    catch (boost::exception &e)                  { VAR_ERROR(boost::diagnostic_information(e)); throw; }
    catch (std::exception& e)                    { VAR_ERROR(e.what());                         throw; }
    catch (...)                                  { LOG_ERROR;                                   throw; }
}

}}} //namespace

BOOST_CLASS_EXPORT_IMPLEMENT(model::video::transition::WipeBarnDoors)
