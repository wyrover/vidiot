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

#include "VideoTransition_WipeCircle.h"

#include "TransitionParameterBool.h"
#include "TransitionParameterInt.h"

namespace model { namespace video { namespace transition {

//////////////////////////////////////////////////////////////////////////
// PARAMETERS
//////////////////////////////////////////////////////////////////////////

wxString WipeCircle::sParameterCount{ "count" };
wxString WipeCircle::sParameterInverse{ "inversed" };
wxString WipeCircle::sParameterSoftenEdges{ "smooth" };

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

WipeCircle* WipeCircle::clone() const
{
    return new WipeCircle(static_cast<const WipeCircle&>(*this));
}

//////////////////////////////////////////////////////////////////////////
// TRANSITION
//////////////////////////////////////////////////////////////////////////

bool WipeCircle::supports(TransitionType type) const
{
    return
        type == TransitionTypeFadeInFromPrevious ||
        type == TransitionTypeFadeOutToNext;
}

std::vector<std::tuple<wxString, wxString, TransitionParameterPtr>> WipeCircle::getParameters() const
{
    return
    {
        std::make_tuple(sParameterCount, _("Number of circles"), boost::make_shared<TransitionParameterInt>(1, 1, 100)),     
        std::make_tuple(sParameterInverse, _("Inversed"), boost::make_shared<TransitionParameterBool>(false)),
        std::make_tuple(sParameterSoftenEdges, _("Soften edges"), boost::make_shared<TransitionParameterBool>(true)),
    };
}

wxString WipeCircle::getDescription(TransitionType type) const
{
    return _("Wipe Circle");
}

//////////////////////////////////////////////////////////////////////////
// VIDEOTRANSITIONOPACITY
//////////////////////////////////////////////////////////////////////////

std::function<float (int,int)> WipeCircle::getRightMethod(const wxImagePtr& image, const float& factor) const
{
    int nBands{ getParameter<TransitionParameterInt>(sParameterCount)->getValue() };
    bool inverse{ getParameter<TransitionParameterBool>(sParameterInverse)->getValue() };
    bool soften{ getParameter<TransitionParameterBool>(sParameterSoftenEdges)->getValue() };
    int w{ image->GetWidth() };
    int h{ image->GetHeight() };
    int bandsize{ euclidianDistance(w / 2, h / 2, 0, 0) / nBands };
    return [bandsize, factor, w, h, inverse, soften](int x, int y) -> float
    {
        return getFactor(bandsize, euclidianDistance(w / 2, h / 2, x, y) % bandsize, factor, inverse, soften);
    };
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void WipeCircle::serialize(Archive & ar, const unsigned int version)
{
    try
    {
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(VideoTransition);
    }
    catch (boost::exception &e)                  { VAR_ERROR(boost::diagnostic_information(e)); throw; }
    catch (std::exception& e)                    { VAR_ERROR(e.what());                         throw; }
    catch (...)                                  { LOG_ERROR;                                   throw; }
}
template void WipeCircle::serialize<boost::archive::xml_oarchive>(boost::archive::xml_oarchive& ar, const unsigned int archiveVersion);
template void WipeCircle::serialize<boost::archive::xml_iarchive>(boost::archive::xml_iarchive& ar, const unsigned int archiveVersion);

}}} //namespace

BOOST_CLASS_EXPORT_IMPLEMENT(model::video::transition::WipeCircle)
