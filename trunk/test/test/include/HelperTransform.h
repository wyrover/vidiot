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

#pragma once

#include "UtilEnumSelector.h"

namespace test {

template <class ITEMTYPE>
void ClickOnEnumSelector(EnumSelector<ITEMTYPE>* widget, ITEMTYPE value)
{
    MouseClickTopLeft(widget);
    KeyboardKeyPress(WXK_HOME);
    for (int step = 0; step < widget->getIndex(value); ++step)
    {
        KeyboardKeyPress(WXK_DOWN);
    }
    KeyboardKeyPress(WXK_RETURN);
    ASSERT_EQUALS(widget->getValue(),value);
    WaitForIdle;
}

void ResizeClip(model::IClipPtr clip, wxSize boundingbox);

} // namespace
