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

#include "TransitionParameterDirection.h"
#include "TransitionParameterInt.h"

namespace model { namespace video { namespace transition {

//////////////////////////////////////////////////////////////////////////
// PARAMETERS
//////////////////////////////////////////////////////////////////////////

wxString Bands::sParameterCount{ "count" };
wxString Bands::sParameterDirection{ "direction" };

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

// todo rename to StraightWipe
Bands* Bands::clone() const
{
    return new Bands(static_cast<const Bands&>(*this));   // todo use BaseCRTP pattern from http://stackoverflow.com/questions/12255546/c-deep-copying-a-base-class-pointer
}

//////////////////////////////////////////////////////////////////////////
// TRANSITION
//////////////////////////////////////////////////////////////////////////

bool Bands::supports(TransitionType type) const
{
    return
        type == TransitionTypeFadeInFromPrevious ||
        type == TransitionTypeFadeOutToNext;
}

std::vector<std::tuple<wxString, wxString, TransitionParameterPtr>> Bands::getParameters() const
{
    return
    {
        std::make_tuple(sParameterCount, _("Number of bands"), boost::make_shared<TransitionParameterInt>(1, 1, 100)),
        std::make_tuple(sParameterDirection, _("Direction"), boost::make_shared<TransitionParameterDirection>(DirectionLeftToRight)),
    };
}

wxString Bands::getDescription(TransitionType type) const
{
    return _("Swipe Straight");
}

//////////////////////////////////////////////////////////////////////////
// VIDEOTRANSITIONOPACITY
//////////////////////////////////////////////////////////////////////////

// todo "clock swipe"
// todo show preview of transition when editing?
// todo in edit clip show a drop down box for changing the transition type.
// todo all parameters of transitions have configurable defaults (extra tab in options)

/// \param w width of image
/// \param h height of image
/// \param a 'a' part of equation describing the diagonal
/// \param b 'b' part of equation describing the diagonal
/// \param nBands number of bands
/// \param factor how 'far' along is the transition?
/// \param reversed_animation if true, start the animation from the 'other end' of a band
std::function<float(int, int)> getDiagonalMethod(double w, double h, double a, double b, int nBands, double factor, bool reversed_animation)
{
    double diagonal_length{ pythagoras(w, h) };
    int bandSize{ static_cast<int>(std::floor(diagonal_length) / nBands) };
    double a_inverse = 1 / a;
    bool cross_diagonal{ a < 0 }; // false: diagonal animation axis is from top-left to bottom-right. true: axis is from bottom-left to top-right.
    double Q{ (cross_diagonal ? -1 : 1) * (w * h) / (h * h + w * w) };
    return [w, h, a, b, factor, cross_diagonal, reversed_animation, bandSize, a_inverse, Q](int x0, int y0) -> float
    {
        double x_intersect{ (a_inverse * static_cast<double>(x0) + static_cast<double>(y0) - b) * Q };
        double y_intersect{ a * x_intersect + b };
        int length_intersect{ static_cast<int>(std::floor(pythagoras(x_intersect, cross_diagonal ? h - y_intersect : y_intersect))) };
        int distance{ length_intersect % bandSize };
        return getFactor(bandSize, distance, factor, reversed_animation);
    };
}

std::function<float (int,int)> Bands::getRightMethod(const wxImagePtr& image, const float& factor) const
{
    int nBands{ getParameter<TransitionParameterInt>(sParameterCount)->getValue() };
    Direction direction{ getParameter<TransitionParameterDirection>(sParameterDirection)->getValue() };

    double w{ static_cast<double>(image->GetWidth()) };
    double h{ static_cast<double>(image->GetHeight()) };
    double diagonal_length{ pythagoras(w, h) };

    switch (direction)
    {
        case DirectionLeftToRight:
        case DirectionRightToLeft:
        {
            int bandsize{ image->GetWidth() / nBands };
            return [bandsize, factor, direction](int x, int y) -> float
            {
                return getFactor(bandsize, x % bandsize, factor, direction == DirectionRightToLeft);
            };
        }
        case DirectionTopToBottom:
        case DirectionBottomToTop:
        {
            int bandsize{ image->GetHeight() / nBands };
            return [bandsize, factor, direction](int x, int y) -> float
            {
                return getFactor(bandsize, y % bandsize, factor, direction == DirectionBottomToTop);
            };
        }
        case DirectionTopLeftToBottomRight:
        case DirectionBottomRightToTopLeft:
        {
            // Normal slope (along this line the length is checked): 
            //
            // y = ax + b
            //
            //      h                 h
            // y = --- * x + b  (a = ---) (b = 0)
            //      w                 w
            //
            // Perpendicular line through (x0,y0):
            //                1                                       w                                         w                    w * h
            // y - y0 = -1 * --- * (x - x0)    <=>     y - y0 = -1 * --- * (x - x0)            <=>       x = ( --- * x0 + y0 ) * ( ---------  )
            //                a                                       h                                         h                  w^2 + h^2
            //
            // (x_intersect, y_intersect) is the point on the image diagonal (top left to bottom right) where the perpendicular line also goes through (x0,y0)
            // That intersect point is then used to determine the 'distance traveled so far'.
            // Note that the distance traveled so far is determined using the top left point of the image as starting point.
            //
            // DirectionBottomRightToTopLeft: same algo (goes 'along' the same line), except with a reversed animation.
            double a{ h / w };
            double b{ 0 };
            return getDiagonalMethod(w, h, a, b, nBands, factor, direction == DirectionBottomRightToTopLeft);
            break;
        }
        case DirectionBottomLeftToTopRight:
        case DirectionTopRightToBottomLeft:
        {
            // Normal slope (along this line the length is checked): 
            //
            // y = ax + b
            //
            //           h                      h
            // y = -1 * --- * x + b  (a = -1 * ---) (b = h)
            //           w                      w
            //
            // Perpendicular line through (x0,y0):
            //                1                                   w                                              w                        w * h
            // y - y0 = -1 * ---  * (x - x0)    <=>     y - y0 = --- * (x - x0)            <=>       x = (-1 *  --- * x0 + y0 - h) * ( -----------  )
            //                a                                   h                                              h                     - h^2 - w^2
            //
            // (x_intersect, y_intersect) is the point on the image diagonal (bottom left to top right) where the perpendicular line also goes through (x0,y0)
            // That intersect point is then used to determine the 'distance traveled so far'.
            // Note that the distance traveled so far is determined using the bottom left point of the image as starting point.
            // 
            // DirectionTopRightToBottomLeft: same algo (goes 'along' the same line), except with a reversed animation.
            double a{ -1 * h / w };
            double b{ h };
            return getDiagonalMethod(w, h, a, b, nBands, factor, direction == DirectionTopRightToBottomLeft);
            break;
        }
    }
    FATAL("Wrong direction");
    return nullptr;
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