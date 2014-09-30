// Copyright 2013,2014 Eric Raijmakers.
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

#ifndef HELPER_TRIM_H
#define HELPER_TRIM_H

namespace test {

/// Trim the given clip on the left side
void TimelineTrimLeft(model::IClipPtr clip, pixel length, bool shift = true, bool endtrim = true);

/// Trim the given clip on the right side
void TimelineTrimRight(model::IClipPtr clip, pixel length, bool shift = true, bool endtrim = true);

void TimelineTrimTransitionLeftClipEnd(model::IClipPtr transition, pixel length, bool shift = true, bool endtrim = true);

void TimelineTrimTransitionRightClipBegin(model::IClipPtr transition, pixel length, bool shift = true, bool endtrim = true);

void TimelineBeginTrim(wxPoint from, bool shift);

void TimelineEndTrim(bool shift = true);

/// Do a trim between the two points (press, move, release). This basically does the same as 'Drag' but faster. The Drag
/// method does the move in several (10) steps. This method simply moves to the begin point, presses the mouse, moves
/// to the end point (without intermediate points) and releases the button.
/// \param from starting position to move to initially
/// \param to final position to drag to
/// \param shift hold down shift after pressing the mouse button
void TimelineTrim(wxPoint from, wxPoint to, bool shift = false, bool endtrim = true);

/// \see TimelineTrim
/// Do a shift trim
void TimelineShiftTrim(wxPoint from, wxPoint to);

} // namespace

#endif