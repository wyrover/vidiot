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

#ifndef UTIL_LOG_WXWIDGETS_H
#define UTIL_LOG_WXWIDGETS_H

std::ostream& operator<<(std::ostream& os, const wxFileName& obj);
std::ostream& operator<<(std::ostream& os, const wxPoint& obj);
std::ostream& operator<<(std::ostream& os, const wxSize& obj);
std::ostream& operator<<(std::ostream& os, const wxRect& obj);
std::ostream& operator<<(std::ostream& os, const wxColour& obj);
std::ostream& operator<<(std::ostream& os, const wxKeyCode& obj);
std::ostream& operator<<(std::ostream& os, const wxKeyEvent& obj);
std::ostream& operator<<(std::ostream& os, const wxMouseEvent& obj);
std::ostream& operator<<(std::ostream& os, const wxKeyboardState& obj);
std::ostream& operator<<(std::ostream& os, const wxRegion& obj);
std::ostream& operator<<(std::ostream& os, const wxCompositionMode& obj);
std::ostream& operator<<(std::ostream& os, const wxInterpolationQuality& obj);
std::ostream& operator<<(std::ostream& os, const wxAntialiasMode& obj);
std::ostream& operator<<(std::ostream& os, const wxGraphicsContext& obj);
std::ostream& operator<<(std::ostream& os, const wxArrayString& obj);
std::ostream& operator<<(std::ostream& os, const wxDateTime& obj);

#endif
