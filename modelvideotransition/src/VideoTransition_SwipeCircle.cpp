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

#include "VideoTransition_SwipeCircle.h"

#include "TransitionParameterBool.h"
#include "TransitionParameterInt.h"

namespace model { namespace video { namespace transition {

//////////////////////////////////////////////////////////////////////////
// PARAMETERS
//////////////////////////////////////////////////////////////////////////

wxString SwipeCircle::sParameterCount{ "count" };
wxString SwipeCircle::sParameterInverse{ "inversed" };

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

SwipeCircle* SwipeCircle::clone() const
{
    return new SwipeCircle(static_cast<const SwipeCircle&>(*this));
}

// todo play with changing the inversed flag, and redoing. sometimes the detailsclip is outofsync with the actual value!

//////////////////////////////////////////////////////////////////////////
// TRANSITION
//////////////////////////////////////////////////////////////////////////

bool SwipeCircle::supports(TransitionType type) const
{
    return
        type == TransitionTypeFadeInFromPrevious ||
        type == TransitionTypeFadeOutToNext;
}

std::vector<std::tuple<wxString, wxString, TransitionParameterPtr>> SwipeCircle::getParameters() const
{
    return
    {
        std::make_tuple(sParameterCount, _("Number of circles"), boost::make_shared<TransitionParameterInt>(1, 1, 100)),     
        std::make_tuple(sParameterInverse, _("Inversed"), boost::make_shared<TransitionParameterBool>(false)),
    };
}

wxString SwipeCircle::getDescription(TransitionType type) const
{
    return _("Swipe Circle");
}

//////////////////////////////////////////////////////////////////////////
// VIDEOTRANSITIONOPACITY
//////////////////////////////////////////////////////////////////////////

std::function<float (int,int)> SwipeCircle::getRightMethod(const wxImagePtr& image, const float& factor) const
{
    int nBands{ getParameter<TransitionParameterInt>(sParameterCount)->getValue() };
    bool inverse{ getParameter<TransitionParameterBool>(sParameterInverse)->getValue() };
    int w{ image->GetWidth() };
    int h{ image->GetHeight() };
    int bandsize{ static_cast<int>(std::max(w, h)) / nBands };
    return [bandsize, factor, w, h, inverse](int x, int y) -> float
    {
        return getFactor(bandsize, distance(w / 2, h / 2, x, y) % bandsize, factor, inverse);
    };
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void SwipeCircle::serialize(Archive & ar, const unsigned int version)
{
    try
    {
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(VideoTransition);
    }
    catch (boost::exception &e)                  { VAR_ERROR(boost::diagnostic_information(e)); throw; }
    catch (std::exception& e)                    { VAR_ERROR(e.what());                         throw; }
    catch (...)                                  { LOG_ERROR;                                   throw; }
}
template void SwipeCircle::serialize<boost::archive::xml_oarchive>(boost::archive::xml_oarchive& ar, const unsigned int archiveVersion);
template void SwipeCircle::serialize<boost::archive::xml_iarchive>(boost::archive::xml_iarchive& ar, const unsigned int archiveVersion);

}}} //namespace

BOOST_CLASS_EXPORT_IMPLEMENT(model::video::transition::SwipeCircle)