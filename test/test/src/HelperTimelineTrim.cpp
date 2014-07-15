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

#include "Test.h"

namespace test {

void TrimLeft(model::IClipPtr clip, pixel length, bool shift, bool endtrim)
{
    wxPoint from = LeftCenter(clip);
    wxPoint to = from;
    to.x += length;
    Trim(from,to,shift,endtrim);
}

void TrimRight(model::IClipPtr clip, pixel length, bool shift, bool endtrim)
{
    wxPoint from = RightCenter(clip);
    wxPoint to = from;
    to.x += length;
    Trim(from,to,shift,endtrim);
}

void BeginTrim(wxPoint from, bool shift)
{
    Move(from);
    waitForIdle();
    if (shift)
    {
        ShiftDown();
        waitForIdle();
    }
    LeftDown();
    waitForIdle();
}

void EndTrim(bool shift)
{
    LeftUp();
    waitForIdle();
    if (shift)
    {
        ShiftUp();
        waitForIdle();
    }
}

void Trim(wxPoint from, wxPoint to, bool shift, bool endtrim)
{
    BeginTrim(from,shift);
    Move(to);
    waitForIdle();
    if (endtrim)
    {
        EndTrim(shift);
    }
}

void ShiftTrim(wxPoint from, wxPoint to)
{
    Trim(from,to,true,true);
}

} // namespace