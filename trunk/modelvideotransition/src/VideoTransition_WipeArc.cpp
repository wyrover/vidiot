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

#include "VideoTransition_WipeArc.h"

#include "TransitionParameterBool.h"
#include "TransitionParameterDirection.h"
#include "TransitionParameterInt.h"

namespace model { namespace video { namespace transition {

//////////////////////////////////////////////////////////////////////////
// PARAMETERS
//////////////////////////////////////////////////////////////////////////

wxString WipeArc::sParameterCount{ "count" };
wxString WipeArc::sParameterDirection{ "direction" };
wxString WipeArc::sParameterInverse{ "inversed" };
wxString WipeArc::sParameterSoftenEdges{ "softenedges" };

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

WipeArc* WipeArc::clone() const
{
    return new WipeArc(static_cast<const WipeArc&>(*this));
}

//////////////////////////////////////////////////////////////////////////
// TRANSITION
//////////////////////////////////////////////////////////////////////////

bool WipeArc::supports(TransitionType type) const
{
    return
        type == TransitionTypeFadeInFromPrevious ||
        type == TransitionTypeFadeOutToNext;
}

std::vector<std::tuple<wxString, wxString, TransitionParameterPtr>> WipeArc::getParameters() const
{
    return
    {
        std::make_tuple(sParameterCount, _("Number of bands"), boost::make_shared<TransitionParameterInt>(1, 1, 100)),
        std::make_tuple(sParameterDirection, _("Direction"), boost::make_shared<TransitionParameterDirection>(DirectionLeftToRight)),
        std::make_tuple(sParameterInverse, _("Inversed"), boost::make_shared<TransitionParameterBool>(false)),
        std::make_tuple(sParameterSoftenEdges, _("Soften edges"), boost::make_shared<TransitionParameterBool>(true)),
    };
}

wxString WipeArc::getDescription(TransitionType type) const
{
    return _("Wipe Arc");
}

//////////////////////////////////////////////////////////////////////////
// VIDEOTRANSITIONOPACITY
//////////////////////////////////////////////////////////////////////////

std::function<float (int,int)> WipeArc::getRightMethod(const wxImagePtr& image, const float& factor) const
{
    int nBands{ getParameter<TransitionParameterInt>(sParameterCount)->getValue() };
    Direction direction{ getParameter<TransitionParameterDirection>(sParameterDirection)->getValue() };
    bool inverse{ getParameter<TransitionParameterBool>(sParameterInverse)->getValue() };
    bool soften{ getParameter<TransitionParameterBool>(sParameterSoftenEdges)->getValue() };
    int w{ image->GetWidth() };
    int h{ image->GetHeight() };
    int diagonal_length{ static_cast<int>(std::floor(pythagoras(w, h))) };
    int x_origin{ 0 };
    int y_origin{ 0 };
    switch (direction)
    {
        case DirectionLeftToRight: { x_origin = 0; y_origin = h / 2; break; }
        case DirectionRightToLeft: { x_origin = w; y_origin = h / 2; break; }
        case DirectionTopToBottom: { x_origin = w / 2; y_origin = 0; break; }
        case DirectionBottomToTop: { x_origin = w / 2; y_origin = h; break; }
        case DirectionTopLeftToBottomRight: { x_origin = 0; y_origin = 0; break; }
        case DirectionBottomRightToTopLeft: { x_origin = w; y_origin = h; break; }
        case DirectionBottomLeftToTopRight: { x_origin = 0; y_origin = h; break; }
        case DirectionTopRightToBottomLeft: { x_origin = w; y_origin = 0; break; }
    }
    // Use the 'farthest away' point to determine the band size. 
    // Using a smaller size gives artifacts.
    // Example: Set nBands to 1, and use the left to right direction.
    //          Pixels farther away than width of image are shown too soon.
    int bandsize{ static_cast<int>(std::floor(diagonal_length)) / nBands };
    return [inverse, soften, bandsize, factor, direction, w, h, x_origin, y_origin](int x, int y) -> float
    {
        return getFactor(bandsize, euclidianDistance(x_origin, y_origin, x, y) % bandsize, factor, inverse, soften);
    };
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void WipeArc::serialize(Archive & ar, const unsigned int version)
{
    try
    {
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(VideoTransition);
    }
    catch (boost::exception &e)                  { VAR_ERROR(boost::diagnostic_information(e)); throw; }
    catch (std::exception& e)                    { VAR_ERROR(e.what());                         throw; }
    catch (...)                                  { LOG_ERROR;                                   throw; }
}
template void WipeArc::serialize<boost::archive::xml_oarchive>(boost::archive::xml_oarchive& ar, const unsigned int archiveVersion);
template void WipeArc::serialize<boost::archive::xml_iarchive>(boost::archive::xml_iarchive& ar, const unsigned int archiveVersion);

}}} //namespace

BOOST_CLASS_EXPORT_IMPLEMENT(model::video::transition::WipeArc)
