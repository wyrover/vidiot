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
inline double pythagoras(double x, double y)
{
    return std::sqrt(x * x + y * y);
};

/// Determine the distance between two points
inline int distance(int x_origin, int y_origin, int x, int y)
{
    return static_cast<int>(std::floor(pythagoras(x_origin - x, y_origin - y)));
};

/// Compute the factor, given a bandsize and a length.
/// \param bandsize length of the total band
/// \param distance part of the band that is covered currently for a given point
/// \param factor transition progress (0 < factor < 1)
/// \param reversed_animation if true, reverse the result
/// \return opacity factor to be applied
inline float getFactor(int bandsize, int distance, double factor, bool reversed_animation)
{
    if (reversed_animation)
    {
        distance = bandsize - distance;
    }
    return (distance <= factor * bandsize) ? 1.0 : 0.0; // todo also make bands which return the not 0 and 1 but the quotient
}

}}} // namespace
