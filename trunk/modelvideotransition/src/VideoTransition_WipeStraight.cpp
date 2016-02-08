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

#include "VideoTransition_WipeStraight.h"

#include "TransitionParameterDirection8.h"
#include "TransitionParameterInt.h"

namespace model { namespace video { namespace transition {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

WipeStraight* WipeStraight::clone() const
{
    return new WipeStraight(static_cast<const WipeStraight&>(*this));
}

//////////////////////////////////////////////////////////////////////////
// TRANSITION
//////////////////////////////////////////////////////////////////////////

bool WipeStraight::supports(TransitionType type) const
{
    return
        type == TransitionTypeFadeInFromPrevious ||
        type == TransitionTypeFadeOutToNext;
}

ParameterAttributes WipeStraight::getAvailableParameters() const
{
    return
    {
        { TransitionParameterInt::sParameterBandsCount, _("Number of lines"), _("Select the number of simultaneous wipes."), boost::make_shared<TransitionParameterInt>(1, 1, 100) },
        { TransitionParameterDirection8::sParameterDirection8, _("Direction"), _("Select the direction of the wipes."), boost::make_shared<TransitionParameterDirection8>(Direction8LeftToRight) },
        { TransitionParameterBool::sParameterSoftenEdges, _("Soften edges"), _("Select to enable smoothing at the edges."), boost::make_shared<TransitionParameterBool>(true) },
    };
}

wxString WipeStraight::getDescription(TransitionType type) const
{
    return _("Wipe Straight");
}

//////////////////////////////////////////////////////////////////////////
// VIDEOTRANSITIONOPACITY
//////////////////////////////////////////////////////////////////////////

/// \param w width of image
/// \param h height of image
/// \param a 'a' part of equation describing the diagonal
/// \param b 'b' part of equation describing the diagonal
/// \param nWipeStraight number of WipeStraight
/// \param factor how 'far' along is the transition?
/// \param reversed_animation if true, start the animation from the 'other end' of a band
std::function<float(int, int)> getDiagonalMethod(double w, double h, double a, double b, int nWipeStraight, double factor, bool reversed_animation, bool smooth)
{
    double diagonal_length{ pythagoras(w, h) };
    int WipeStraightize{ static_cast<int>(std::floor(diagonal_length) / nWipeStraight) };
    double a_inverse = 1 / a;
    bool cross_diagonal{ a < 0 }; // false: diagonal animation axis is from top-left to bottom-right. true: axis is from bottom-left to top-right.
    double Q{ (cross_diagonal ? -1 : 1) * (w * h) / (h * h + w * w) };
    return [w, h, a, b, factor, cross_diagonal, reversed_animation, smooth, WipeStraightize, a_inverse, Q](int x0, int y0) -> float
    {
        double x_intersect{ (a_inverse * static_cast<double>(x0) + static_cast<double>(y0) - b) * Q };
        double y_intersect{ a * x_intersect + b };
        int length_intersect{ static_cast<int>(std::floor(pythagoras(x_intersect, cross_diagonal ? h - y_intersect : y_intersect))) };
        int distance{ length_intersect % WipeStraightize };
        return getFactor(WipeStraightize, distance, factor, reversed_animation, smooth);
    };
}

std::function<float (int,int)> WipeStraight::getRightMethod(const wxImagePtr& image, const float& factor) const
{
    int nWipeStraight{ getParameter<TransitionParameterInt>(TransitionParameterInt::sParameterBandsCount)->getValue() };
    Direction8 direction{ getParameter<TransitionParameterDirection8>(TransitionParameterDirection8::sParameterDirection8)->getValue() };
    bool soften{ getParameter<TransitionParameterBool>(TransitionParameterBool::sParameterSoftenEdges)->getValue() };

    double w{ static_cast<double>(image->GetWidth()) };
    double h{ static_cast<double>(image->GetHeight()) };
    double diagonal_length{ pythagoras(w, h) };

    switch (direction)
    {
        case Direction8LeftToRight:
        case Direction8RightToLeft:
        {
            int WipeStraightize{ image->GetWidth() / nWipeStraight };
            return [WipeStraightize, factor, direction, soften](int x, int y) -> float
            {
                return getFactor(WipeStraightize, x % WipeStraightize, factor, direction == Direction8RightToLeft, soften);
            };
        }
        case Direction8TopToBottom:
        case Direction8BottomToTop:
        {
            int WipeStraightize{ image->GetHeight() / nWipeStraight };
            return [WipeStraightize, factor, direction, soften](int x, int y) -> float
            {
                return getFactor(WipeStraightize, y % WipeStraightize, factor, direction == Direction8BottomToTop, soften);
            };
        }
        case Direction8TopLeftToBottomRight:
        case Direction8BottomRightToTopLeft:
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
            return getDiagonalMethod(w, h, a, b, nWipeStraight, factor, direction == Direction8BottomRightToTopLeft, soften);
            break;
        }
        case Direction8BottomLeftToTopRight:
        case Direction8TopRightToBottomLeft:
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
            return getDiagonalMethod(w, h, a, b, nWipeStraight, factor, direction == Direction8TopRightToBottomLeft, soften);
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
void WipeStraight::serialize(Archive & ar, const unsigned int version)
{
    try
    {
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(VideoTransition);
    }
    catch (boost::exception &e)                  { VAR_ERROR(boost::diagnostic_information(e)); throw; }
    catch (std::exception& e)                    { VAR_ERROR(e.what());                         throw; }
    catch (...)                                  { LOG_ERROR;                                   throw; }
}
template void WipeStraight::serialize<boost::archive::xml_oarchive>(boost::archive::xml_oarchive& ar, const unsigned int archiveVersion);
template void WipeStraight::serialize<boost::archive::xml_iarchive>(boost::archive::xml_iarchive& ar, const unsigned int archiveVersion);

template<class Archive>
void Bands::serialize(Archive & ar, const unsigned int version)
{
    try
    {
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(VideoTransition);
    }
    catch (boost::exception &e) { VAR_ERROR(boost::diagnostic_information(e)); throw; }
    catch (std::exception& e) { VAR_ERROR(e.what());                         throw; }
    catch (...) { LOG_ERROR;                                   throw; }
}

}}} //namespace

BOOST_CLASS_EXPORT_IMPLEMENT(model::video::transition::WipeStraight)
BOOST_CLASS_EXPORT_IMPLEMENT(model::video::transition::Bands)
