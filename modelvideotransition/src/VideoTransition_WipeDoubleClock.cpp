// Copyright 2016 Eric Raijmakers.
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

#include "VideoTransition_WipeDoubleClock.h"

#include "TransitionParameterBool.h"
#include "TransitionParameterInt.h"

namespace model { namespace video { namespace transition {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

WipeDoubleClock* WipeDoubleClock::clone() const
{
    return new WipeDoubleClock(static_cast<const WipeDoubleClock&>(*this));
}

//////////////////////////////////////////////////////////////////////////
// TRANSITION
//////////////////////////////////////////////////////////////////////////

bool WipeDoubleClock::supports(TransitionType type) const
{
    return true;
        //type == TransitionTypeFadeInFromPrevious ||
        //type == TransitionTypeFadeOutToNext;
}

ParameterAttributes WipeDoubleClock::getAvailableParameters() const
{
    return
    {
        { TransitionParameterInt::sParameterAngle, _("Angle"), _("Select the starting angle."), "arrow-circle-double-135.png", boost::make_shared<TransitionParameterInt>(0, 0, 360) },
        { TransitionParameterBool::sParameterSoftenEdges, _("Soften edges"), _("Select to enable smoothing at the edges."), "cushion-blue.png", boost::make_shared<TransitionParameterBool>(true) },
    };
}

wxString WipeDoubleClock::getDescription(TransitionType type) const
{
    return _("Wipe Double Clock");
}

//////////////////////////////////////////////////////////////////////////
// VIDEOTRANSITIONOPACITY
//////////////////////////////////////////////////////////////////////////

std::function<float (int,int)> WipeDoubleClock::getRightMethod(const wxImagePtr& image, const float& factor) const
{
    int angle{ getParameter<TransitionParameterInt>(TransitionParameterInt::sParameterAngle)->getValue() };
    bool soften{ getParameter<TransitionParameterBool>(TransitionParameterBool::sParameterSoftenEdges)->getValue() };
    int x_origin{ image->GetWidth() / 2};
    int y_origin{ image->GetHeight() / 2};

    auto calcdist = [angle, x_origin, y_origin](int x, int y) -> int
    {
        int r{ angularDistance(x_origin, y_origin, x, y) - angle };
        if (r < 0) { r += 360; }
        if (r > 180) { r = 360 - r; }
        return r;
    };

    return [factor, calcdist, soften](int x, int y) -> float
    {
        return getFactor(180, calcdist(x,y), factor, false, soften);
    };
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void WipeDoubleClock::serialize(Archive & ar, const unsigned int version)
{
    try
    {
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(VideoTransition);
    }
    catch (boost::exception &e)                  { VAR_ERROR(boost::diagnostic_information(e)); throw; }
    catch (std::exception& e)                    { VAR_ERROR(e.what());                         throw; }
    catch (...)                                  { LOG_ERROR;                                   throw; }
}
template void WipeDoubleClock::serialize<boost::archive::xml_oarchive>(boost::archive::xml_oarchive& ar, const unsigned int archiveVersion);
template void WipeDoubleClock::serialize<boost::archive::xml_iarchive>(boost::archive::xml_iarchive& ar, const unsigned int archiveVersion);

}}} //namespace

BOOST_CLASS_EXPORT_IMPLEMENT(model::video::transition::WipeDoubleClock)