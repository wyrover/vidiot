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

#include "VideoTransitionHelpers.h"

namespace model { namespace video { namespace transition {

/// Given two sides of a triangle, compute the hypothenusa
/// \param x side 1
/// \param y side 2
/// \return length of hypothenusa
double pythagoras(double x, double y)
{
    return std::sqrt(x * x + y * y);
};

/// Determine the distance between two points
int euclidianDistance(int x_origin, int y_origin, int x, int y)
{
    return static_cast<int>(std::floor(pythagoras(x_origin - x, y_origin - y)));
};

int angularDistance(int x_origin, int y_origin, int x, int y)
{
    int diffx{ x - x_origin };
    int diffy{ y_origin - y };
    int result{ 0 };
    if (diffx != 0 || diffy != 0)
    {
        result = static_cast<int>(std::floor(std::atan2(diffx, diffy) * 180 / M_PI));
    }
    while (result < 0) { result += 360; }
    while (result >= 360) { result -= 360; }
    return result;
}

/// Compute the factor, given a bandsize and a length.
/// \param bandsize length of the total band
/// \param distance distance of a given point wrt transition progress
/// \param factor transition progress (0 < factor < 1)
/// \param reversed_animation if true, reverse the result
/// \param smooth apply smoothing if true
/// \return opacity factor to be applied
float getFactor(int bandsize, int distance, double factor, bool reversed_animation, bool smooth)
{
    if (reversed_animation)
    {
        distance = bandsize - distance;
    }

    if (smooth)
    {
        double smooth_factor{ 0.01 }; // 0.01 == 1%

        // Slow the transition a bit such that the smoothed area gets time to 'fade in'.
        // Normally, the first fully opaque pixels area shown in the first frame.
        // However, I want the smoothed area also to enter gradually.
        // Without this, when the first transition frame is shown, the entire smooted area is shown, immediately.
        double distance_with_smooth_in_front{ (double)distance + (double)bandsize * smooth_factor };

        // Speed up the transition such that the end time remains the same.
        factor *= 1.0 + smooth_factor;

        double end{ factor * (double)bandsize };
        if (distance_with_smooth_in_front <= end)
        {
            return 1.0;
        }

        double smooth_area{ (double)bandsize * smooth_factor };
        double smooth_area_end{ end + smooth_area };
        if (distance_with_smooth_in_front <= smooth_area_end)
        {
            // This is the smoothed region
            return (smooth_area_end - distance_with_smooth_in_front) / smooth_area;
        }
        return 0.0;
    }
    else
    {
        double end{ factor * bandsize };
        return distance <= end ? 1.0 : 0.0;
    }
}

}}} // namespace
