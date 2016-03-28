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
#include "TransitionParameterDirection8.h"
#include "TransitionParameterInt.h"

namespace model { namespace video { namespace transition {

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
    return true;
        //type == TransitionTypeFadeInFromPrevious ||
        //type == TransitionTypeFadeOutToNext;
}

ParameterAttributes WipeArc::getAvailableParameters() const
{
    return
    {
        { TransitionParameterInt::sParameterBandsCount, _("Number of arcs"), _("Select the number of simultaneous wipes.") , boost::make_shared<TransitionParameterInt>(1, 1, 100) },
        { TransitionParameterDirection8::sParameterDirection8, _("Direction"), _("Select the direction of the wipes."), boost::make_shared<TransitionParameterDirection8>(Direction8LeftToRight) },
        { TransitionParameterBool::sParameterInversed, _("Inversed"), _("Select between 'growing' and 'shrinking' (in the opposite direction)."), boost::make_shared<TransitionParameterBool>(false) },
        { TransitionParameterBool::sParameterSoftenEdges, _("Soften edges"), _("Select to enable smoothing at the edges."), boost::make_shared<TransitionParameterBool>(true) },
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
    int nBands{ getParameter<TransitionParameterInt>(TransitionParameterInt::sParameterBandsCount)->getValue() };
    Direction8 direction{ getParameter<TransitionParameterDirection8>(TransitionParameterDirection8::sParameterDirection8)->getValue() };
    bool inverse{ getParameter<TransitionParameterBool>(TransitionParameterBool::sParameterInversed)->getValue() };
    bool soften{ getParameter<TransitionParameterBool>(TransitionParameterBool::sParameterSoftenEdges)->getValue() };
    int w{ image->GetWidth() };
    int h{ image->GetHeight() };
    int diagonal_length{ static_cast<int>(std::floor(pythagoras(w, h))) };
    int x_origin{ 0 };
    int y_origin{ 0 };
    switch (direction)
    {
        case Direction8LeftToRight: { x_origin = 0; y_origin = h / 2; break; }
        case Direction8RightToLeft: { x_origin = w; y_origin = h / 2; break; }
        case Direction8TopToBottom: { x_origin = w / 2; y_origin = 0; break; }
        case Direction8BottomToTop: { x_origin = w / 2; y_origin = h; break; }
        case Direction8TopLeftToBottomRight: { x_origin = 0; y_origin = 0; break; }
        case Direction8BottomRightToTopLeft: { x_origin = w; y_origin = h; break; }
        case Direction8BottomLeftToTopRight: { x_origin = 0; y_origin = h; break; }
        case Direction8TopRightToBottomLeft: { x_origin = w; y_origin = 0; break; }
        default: { FATAL("Wrong direction"); break; }
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
