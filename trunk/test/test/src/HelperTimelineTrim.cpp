// Copyright 2013-2015 Eric Raijmakers.
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

void TimelineTrimLeft(model::IClipPtr clip, pixel length, bool shift, bool endtrim)
{
    wxPoint from = LeftCenter(clip);
    wxPoint to = from;
    to.x += length;
    TimelineTrim(from,to,shift,endtrim);
}

void TimelineTrimRight(model::IClipPtr clip, pixel length, bool shift, bool endtrim)
{
    wxPoint from = RightCenter(clip);
    wxPoint to = from;
    to.x += length;
    TimelineTrim(from,to,shift,endtrim);
}

void TimelineTrimTransitionLeftClipEnd(model::IClipPtr transition, pixel length, bool shift, bool endtrim)
{
    wxPoint p(TransitionLeftClipEnd(transition));
    TimelineTrim(p, p + wxPoint(length,0), shift, endtrim);
}

void TimelineTrimTransitionRightClipBegin(model::IClipPtr transition, pixel length, bool shift, bool endtrim)
{
    wxPoint p(TransitionRightClipBegin(transition));
    TimelineTrim(p, p + wxPoint(length,0), shift, endtrim);
}

void TimelineBeginTrim(wxPoint from, bool shift)
{
    TimelineMove(from);
    WaitForIdle();
    if (shift)
    {
        TimelineKeyDown(WXK_SHIFT);
        WaitForIdle();
    }
    TimelineLeftDown();
    WaitForIdle();
}

void TimelineEndTrim(bool shift)
{
    TimelineLeftUp();
    WaitForIdle();
    if (shift)
    {
        TimelineKeyUp(WXK_SHIFT);
        WaitForIdle();
    }
}

void TimelineTrim(wxPoint from, wxPoint to, bool shift, bool endtrim)
{
    TimelineBeginTrim(from,shift);
    TimelineMove(to);
    WaitForIdle();
    if (endtrim)
    {
        TimelineEndTrim(shift);
    }
}

void TimelineShiftTrim(wxPoint from, wxPoint to)
{
    TimelineTrim(from,to,true,true);
}

} // namespace