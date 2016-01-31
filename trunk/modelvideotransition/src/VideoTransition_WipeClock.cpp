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

#include "VideoTransition_WipeClock.h"

#include "TransitionParameterBool.h"
#include "TransitionParameterInt.h"

namespace model { namespace video { namespace transition {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

WipeClock* WipeClock::clone() const
{
    return new WipeClock(static_cast<const WipeClock&>(*this));
}

//////////////////////////////////////////////////////////////////////////
// TRANSITION
//////////////////////////////////////////////////////////////////////////

bool WipeClock::supports(TransitionType type) const
{
    return
        type == TransitionTypeFadeInFromPrevious ||
        type == TransitionTypeFadeOutToNext;
}

std::vector<std::tuple<wxString, wxString, TransitionParameterPtr>> WipeClock::getAvailableParameters() const
{
    return
    {
        std::make_tuple(TransitionParameterInt::sParameterAngle, _("Angle"), boost::make_shared<TransitionParameterInt>(0, 0, 360)),
        std::make_tuple(TransitionParameterInt::sParameterBandsCount, _("Number of lines"), boost::make_shared<TransitionParameterInt>(1, 1, 100)),
        std::make_tuple(TransitionParameterBool::sParameterInversed, _("Inversed"), boost::make_shared<TransitionParameterBool>(false)),
        std::make_tuple(TransitionParameterBool::sParameterSoftenEdges, _("Soften edges"), boost::make_shared<TransitionParameterBool>(true)),
    };
}

wxString WipeClock::getDescription(TransitionType type) const
{
    return _("Wipe Clock");
}

//////////////////////////////////////////////////////////////////////////
// VIDEOTRANSITIONOPACITY
//////////////////////////////////////////////////////////////////////////

std::function<float (int,int)> WipeClock::getRightMethod(const wxImagePtr& image, const float& factor) const
{
    int angle{ getParameter<TransitionParameterInt>(TransitionParameterInt::sParameterAngle)->getValue() };
    int nBands{ getParameter<TransitionParameterInt>(TransitionParameterInt::sParameterBandsCount)->getValue() };
    bool inverse{ getParameter<TransitionParameterBool>(TransitionParameterBool::sParameterInversed)->getValue() };
    bool soften{ getParameter<TransitionParameterBool>(TransitionParameterBool::sParameterSoftenEdges)->getValue() };
    int x_origin{ image->GetWidth() / 2};
    int y_origin{ image->GetHeight() / 2};
    int bandsize{ 360 / nBands };

    auto calcdist = [angle, x_origin, y_origin](int x, int y) -> int
    {
        int r{ angularDistance(x_origin, y_origin, x, y) - angle };
        if (r < 0) { r += 360; }
        return r;
    };

    return [bandsize, x_origin, y_origin, factor, inverse, calcdist, soften](int x, int y) -> float
    {
        return getFactor(bandsize, calcdist(x, y) % bandsize, factor, inverse, soften);
    };
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void WipeClock::serialize(Archive & ar, const unsigned int version)
{
    try
    {
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(VideoTransition);
    }
    catch (boost::exception &e)                  { VAR_ERROR(boost::diagnostic_information(e)); throw; }
    catch (std::exception& e)                    { VAR_ERROR(e.what());                         throw; }
    catch (...)                                  { LOG_ERROR;                                   throw; }
}
template void WipeClock::serialize<boost::archive::xml_oarchive>(boost::archive::xml_oarchive& ar, const unsigned int archiveVersion);
template void WipeClock::serialize<boost::archive::xml_iarchive>(boost::archive::xml_iarchive& ar, const unsigned int archiveVersion);

}}} //namespace

BOOST_CLASS_EXPORT_IMPLEMENT(model::video::transition::WipeClock)