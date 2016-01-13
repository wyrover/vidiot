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

#include "VideoTransition_Bands.h"

#include "Config.h"
#include "TransitionFactory.h"
#include "TransitionParameterDirection.h"
#include "TransitionParameterInt.h"
#include "UtilVector.h"
#include "VideoClip.h"
#include "VideoCompositionParameters.h"
#include "VideoFrame.h"

namespace model { namespace video { namespace transition {

const wxString sParameterCount{ "count" };
const wxString sParameterDirection{ "direction" };

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

Bands::Bands()
    :	VideoTransitionOpacity()
{
    VAR_DEBUG(this);
    addParameter(sParameterCount, boost::make_shared<TransitionParameterInt>(10));
    addParameter(sParameterDirection, boost::make_shared<TransitionParameterDirection>(DirectionLeftToRight));
}

Bands::Bands(const Bands& other)
    :   VideoTransitionOpacity(other)
{
    VAR_DEBUG(*this);
}

Bands* Bands::clone() const
{
    return new Bands(static_cast<const Bands&>(*this));
}

Bands::~Bands()
{
    VAR_DEBUG(this);
}

//////////////////////////////////////////////////////////////////////////
// TRANSITION
//////////////////////////////////////////////////////////////////////////

void Bands::initParameters()
{
    getParameter<TransitionParameterInt>(sParameterCount)->setRange(1, 100);
}

wxString Bands::getDescription(TransitionType type) const
{
    return _("Bands");
}

wxString Bands::getParameterHumanReadibleName(wxString name) const
{
    std::map<wxString, wxString> sMap
    {
        { sParameterCount, _("Number of bands") },
        { sParameterDirection, _("Direction of bands") },
    };
    ASSERT_MAP_CONTAINS(sMap, name);
    return sMap.find(name)->second;
}

//////////////////////////////////////////////////////////////////////////
// VIDEOTRANSITIONOPACITY
//////////////////////////////////////////////////////////////////////////

void Bands::handleFullyOpaqueImage(const wxImagePtr& image, const std::function<float (int, int)>& f) const
{
    applyToAllPixels(image, f);
}

void Bands::handleImageWithAlpha(const wxImagePtr& image, const std::function<float (int, int)>& f) const
{
    applyToAllPixels(image,f);
}

std::function<float(int, int)> getMethod(const wxImagePtr& image, const float& factor, const Direction& direction, int nBands)
{
    auto pytho = [](double x, double y) -> double
    {
        return std::sqrt(x * x + y * y);
    };

    switch (direction)
    {
        ///////////////////////////////////////////////////////////////////////
        case DirectionLeftToRight:
        {
            int bandSize{ image->GetWidth() / nBands };
            return [bandSize, factor](int x, int y) -> float
            {
                return (x % bandSize <= factor * bandSize) ? 1.0 : 0.0;
            };
        }
        case DirectionRightToLeft:
        {
            int bandSize{ image->GetWidth() / nBands };
            return [bandSize, factor](int x, int y) -> float
            {
                return (bandSize - (x % bandSize) <= factor * bandSize) ? 1.0 : 0.0;
            };
        }
        ///////////////////////////////////////////////////////////////////////
        case DirectionTopToBottom:
        {
            int bandSize{ image->GetHeight() / nBands };
            return [bandSize, factor](int x, int y) -> float
            {
                return (y % bandSize <= factor * bandSize) ? 1.0 : 0.0;   // todo also make bands which return the not 0 and 1 but the quotient
            };
        }
        case DirectionBottomToTop:
        {
            int bandSize{ image->GetHeight() / nBands };
            return [bandSize, factor](int x, int y) -> float
            {
                return (bandSize - (y % bandSize) <= factor * bandSize) ? 1.0 : 0.0;
            };
        }
        ///////////////////////////////////////////////////////////////////////
        case DirectionTopLeftToBottomRight:
        {
            // todo keep this as this gives nice round bands (other transition!)
            //int length{ pytho(image->GetWidth(), image->GetHeight()) };
            //int bandSize{ length / nBands };
            //return [pytho, length, bandSize, factor](int x, int y) -> float
            //{
            //    int l{ pytho(x,y) };
            //    return (bandSize - (l % bandSize) <= factor * bandSize) ? 1.0 : 0.0;
            //};

            // Normal slope (along this line the length is checked): 
            //      h
            // y = --- * x + b  (m = h/w) (b = 0)
            //      w
            //
            // Perpendicular line through (x0,y0):
            //                                                      w                                         w                    w * h
            // y - y0 = -1 * m * (x - x0)    <=>     y - y0 = -1 * --- * (x - x0)            <=>       x = ( --- * x0 + y0 ) * ( ---------  )
            //                                                      h                                         h                  w^2 + h^2
            //
            // The resulting intersect point is the point on the diagonal of the image rectangle where the perpendicular line also goes through (x,y)
            // That point is then used to determine the length.
            double width{ static_cast<double>(image->GetWidth()) };
            double height{ static_cast<double>(image->GetHeight()) };
            double length{ pytho(width, height) };
            int bandSize{ static_cast<int>(std::floor(length) / nBands) };
            double m = height / width;
            double m1 = width / height;
            double Q = (width * height) / (width * width + height * height);
            return [factor, pytho, width, height, length, bandSize, m, m1, Q](int x0, int y0) -> float
            {
                double x_intersect{ (m1 * static_cast<double>(x0) + static_cast<double>(y0)) * Q };
                double y_intersect{ m * x_intersect };
                int dist{ static_cast<int>(std::floor(pytho(x_intersect, y_intersect))) };
                return (dist % bandSize <= factor * bandSize) ? 1.0 : 0.0;
            };
            break;
        }
        case DirectionBottomRightToTopLeft:
        {
            // Exact same algo as DirectionTopLeftToBottomRight (goes 'along' the same line), except with a different return value.
            double width{ static_cast<double>(image->GetWidth()) };
            double height{ static_cast<double>(image->GetHeight()) };
            double length{ pytho(width, height) };
            int bandSize{ static_cast<int>(std::floor(length) / nBands) };
            double m = height / width;
            double m1 = width / height;
            double Q = (width * height) / (width * width + height * height);
            return [factor, pytho, width, height, length, bandSize, m, m1, Q](int x0, int y0) -> float
            {
                double x_intersect{ (m1 * static_cast<double>(x0) + static_cast<double>(y0)) * Q };
                double y_intersect{ m * x_intersect };
                int dist{ static_cast<int>(std::floor(pytho(x_intersect, y_intersect))) };
                return (bandSize - (dist % bandSize) <= factor * bandSize) ? 1.0 : 0.0;
            };
            break;
        }
        ///////////////////////////////////////////////////////////////////////
        case DirectionBottomLeftToTopRight:
        {
            // Almost same algo as DirectionTopLeftToBottomRight:
            // Normal slope (along this line the length is checked): 
            //           h
            // y = -1 * --- * x + b  (m = -1 * h/w) (b = h)
            //           w
            //
            // Perpendicular line through (x0,y0):
            //                                                 w                                              w                    w * h
            // y - y0 = -1 * m * (x - x0)    <=>     y - y0 = --- * (x - x0)            <=>       x = (-1 *  --- * x0 + y0 ) * ( ---------  )
            //                                                 h                                              h                  h^2 - w^2
            //
            // The resulting intersect point is the point on the diagonal of the image rectangle where the perpendicular line also goes through (x,y)
            // That point is then used to determine the length.
            //
            // Furthermore, the 'dist' is determined along the other diagonal of the rectangle.
            double width{ static_cast<double>(image->GetWidth()) };
            double height{ static_cast<double>(image->GetHeight()) };
            double length{ pytho(width, height) };
            int bandSize{ static_cast<int>(std::floor(length) / nBands) };
            double m = -1 * height / width;   // todo rename m to a and then use y = ax + b everywhere and make b 0 for the other cases
            double m1 = -1 * width / height; // 1/m todo add comment in other cases todo rename to m_inverse
            double Q = (width * height) / (- 1* height * height + -1 * width * width); // todo width latest everywhere
            return [factor, pytho, width, height, length, bandSize, m, m1, Q](int x0, int y0) -> float
            {
                double x_intersect{ (m1 * static_cast<double>(x0) + static_cast<double>(y0) - height) * Q };
                double y_intersect{ m * x_intersect + height};
                int dist{ static_cast<int>(std::floor(pytho(x_intersect, height - y_intersect))) };
                return (dist % bandSize <= factor * bandSize) ? 1.0 : 0.0;
            };
            break;
        }
        case DirectionTopRightToBottomLeft:
        {
            // Exact same algo as DirectionTopLeftToBottomRight (goes 'along' the same line), except with a different return value.
            double width{ static_cast<double>(image->GetWidth()) };
            double height{ static_cast<double>(image->GetHeight()) };
            double length{ pytho(width, height) };
            int bandSize{ static_cast<int>(std::floor(length) / nBands) };
            double m = -1 * height / width;   // todo rename m to a and then use y = ax + b everywhere and make b 0 for the other cases
            double m1 = -1 * width / height; // 1/m todo add comment in other cases todo rename to m_inverse
            double Q = (width * height) / (-1 * height * height + -1 * width * width); // todo width latest everywhere
            return [factor, pytho, width, height, length, bandSize, m, m1, Q](int x0, int y0) -> float
            {
                double x_intersect{ (m1 * static_cast<double>(x0) + static_cast<double>(y0) - height) * Q };
                double y_intersect{ m * x_intersect + height };
                int dist{ static_cast<int>(std::floor(pytho(x_intersect, height - y_intersect))) };
                return (bandSize - (dist % bandSize) <= factor * bandSize) ? 1.0 : 0.0;
            };
            break;
        }
    }
    FATAL("Wrong direction");
    return nullptr;
}

std::function<float (int,int)> Bands::getLeftMethod(const wxImagePtr& image, const float& factor) const
{
    int nBands{ getParameter<TransitionParameterInt>(sParameterCount)->getValue() };
    Direction direction{ getParameter<TransitionParameterDirection>(sParameterDirection)->getValue() };
    return [](int x, int y) -> float { return 1.0; };
    //return getMethod(image, factor, direction, nBands);
}

std::function<float (int,int)> Bands::getRightMethod(const wxImagePtr& image, const float& factor) const
{
    int nBands{ getParameter<TransitionParameterInt>(sParameterCount)->getValue() };
    Direction direction{ getParameter<TransitionParameterDirection>(sParameterDirection)->getValue() };
    return getMethod(image, factor, direction, nBands);
    //switch (direction)
    //{
    //    ///////////////////////////////////////////////////////////////////////
    //    case DirectionLeftToRight: return getMethod(image, factor, DirectionRightToLeft, nBands);
    //    case DirectionRightToLeft: return getMethod(image, factor, DirectionLeftToRight, nBands);
    //    ///////////////////////////////////////////////////////////////////////
    //    case DirectionTopToBottom: return getMethod(image, factor, DirectionBottomToTop, nBands);
    //    case DirectionBottomToTop: return getMethod(image, factor, DirectionTopToBottom, nBands);
    //    ///////////////////////////////////////////////////////////////////////
    //    case DirectionTopLeftToBottomRight: return getMethod(image, factor, DirectionBottomRightToTopLeft, nBands);
    //    case DirectionBottomRightToTopLeft: return getMethod(image, factor, DirectionTopLeftToBottomRight, nBands);
    //    ///////////////////////////////////////////////////////////////////////
    //    case DirectionBottomLeftToTopRight: return getMethod(image, factor, DirectionTopRightToBottomLeft, nBands);
    //    case DirectionTopRightToBottomLeft: return getMethod(image, factor, DirectionBottomLeftToTopRight, nBands);
    //}
    //FATAL("Wrong direction");
    //return nullptr;
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, const Bands& obj)
{
    os << static_cast<const VideoTransition&>(obj);
    return os;
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void Bands::serialize(Archive & ar, const unsigned int version)
{
    try
    {
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(VideoTransition);
    }
    catch (boost::exception &e)                  { VAR_ERROR(boost::diagnostic_information(e)); throw; }
    catch (std::exception& e)                    { VAR_ERROR(e.what());                         throw; }
    catch (...)                                  { LOG_ERROR;                                   throw; }
}
template void Bands::serialize<boost::archive::xml_oarchive>(boost::archive::xml_oarchive& ar, const unsigned int archiveVersion);
template void Bands::serialize<boost::archive::xml_iarchive>(boost::archive::xml_iarchive& ar, const unsigned int archiveVersion);

}}} //namespace

BOOST_CLASS_EXPORT_IMPLEMENT(model::video::transition::Bands)