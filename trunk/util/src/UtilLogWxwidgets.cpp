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

#define LOGENUMVALUE(value) case value: os << #value; break

std::ostream& operator<<(std::ostream& os, const wxFileName& obj)
{
    os << obj.GetPath() << '|' << obj.GetName() << '|' << obj.GetExt();
    return os;
}

std::ostream& operator<<(std::ostream& os, const wxPoint& obj)
{
    os << '(' << std::setfill(' ') << std::setw(6) << obj.x << ',' << std::setw(6) << obj.y << ')';
    return os;
}

std::ostream& operator<<(std::ostream& os, const wxSize& obj)
{
    os  << '('
        << obj.GetWidth()
        << ','
        << obj.GetHeight()
        << ')';
    return os;
}

std::ostream& operator<<(std::ostream& os, const wxRect& obj)
{
    os  << '('
        << obj.GetX()
        << ','
        << obj.GetY()
        << ','
        << obj.GetWidth()
        << ','
        << obj.GetHeight()
        << ')';
    return os;
}

std::ostream& operator<<(std::ostream& os, const wxColour& obj)
{
    os  << '('
        << static_cast<unsigned int>(obj.Red())
        << ','
        << static_cast<unsigned int>(obj.Green())
        << ','
        << static_cast<unsigned int>(obj.Blue())
        << ','
        << static_cast<unsigned int>(obj.Alpha())
        << ')';
    return os;
}

std::ostream& operator<<(std::ostream& os, const wxKeyEvent& obj)
{
    os  << obj.GetX() << '|'
        << obj.GetY() << '|'
        << obj.GetKeyCode() << '|'
        << obj.GetUnicodeKey() << '|'
        << obj.ControlDown() << '|'
        << obj.ShiftDown() << '|'
        << obj.AltDown() << '|'
        << static_cast<const wxKeyboardState&>(obj);
    return os;
}

std::ostream& operator<<(std::ostream& os, const wxMouseEvent& obj)
{
    os  << obj.GetX() << '|'
        << obj.GetY() << '|'
        << static_cast<const wxKeyboardState&>(obj);
    return os;
}

std::ostream& operator<<(std::ostream& os, const wxKeyCode& obj)
{
    switch (obj)
    {
        LOGENUMVALUE(WXK_BACK);
        LOGENUMVALUE(WXK_TAB);
        LOGENUMVALUE(WXK_RETURN);
        LOGENUMVALUE(WXK_ESCAPE);
        LOGENUMVALUE(WXK_SPACE);
        LOGENUMVALUE(WXK_DELETE);
        LOGENUMVALUE(WXK_START);
        LOGENUMVALUE(WXK_LBUTTON);
        LOGENUMVALUE(WXK_RBUTTON);
        LOGENUMVALUE(WXK_CANCEL);
        LOGENUMVALUE(WXK_MBUTTON);
        LOGENUMVALUE(WXK_CLEAR);
        LOGENUMVALUE(WXK_SHIFT);
        LOGENUMVALUE(WXK_ALT);
        LOGENUMVALUE(WXK_CONTROL);
        LOGENUMVALUE(WXK_MENU);
        LOGENUMVALUE(WXK_PAUSE);
        LOGENUMVALUE(WXK_CAPITAL);
        LOGENUMVALUE(WXK_END);
        LOGENUMVALUE(WXK_HOME);
        LOGENUMVALUE(WXK_LEFT);
        LOGENUMVALUE(WXK_UP);
        LOGENUMVALUE(WXK_RIGHT);
        LOGENUMVALUE(WXK_DOWN);
        LOGENUMVALUE(WXK_SELECT);
        LOGENUMVALUE(WXK_PRINT);
        LOGENUMVALUE(WXK_EXECUTE);
        LOGENUMVALUE(WXK_SNAPSHOT);
        LOGENUMVALUE(WXK_INSERT);
        LOGENUMVALUE(WXK_HELP);
        LOGENUMVALUE(WXK_NUMPAD0);
        LOGENUMVALUE(WXK_NUMPAD1);
        LOGENUMVALUE(WXK_NUMPAD2);
        LOGENUMVALUE(WXK_NUMPAD3);
        LOGENUMVALUE(WXK_NUMPAD4);
        LOGENUMVALUE(WXK_NUMPAD5);
        LOGENUMVALUE(WXK_NUMPAD6);
        LOGENUMVALUE(WXK_NUMPAD7);
        LOGENUMVALUE(WXK_NUMPAD8);
        LOGENUMVALUE(WXK_NUMPAD9);
        LOGENUMVALUE(WXK_MULTIPLY);
        LOGENUMVALUE(WXK_ADD);
        LOGENUMVALUE(WXK_SEPARATOR);
        LOGENUMVALUE(WXK_SUBTRACT);
        LOGENUMVALUE(WXK_DECIMAL);
        LOGENUMVALUE(WXK_DIVIDE);
        LOGENUMVALUE(WXK_F1);
        LOGENUMVALUE(WXK_F2);
        LOGENUMVALUE(WXK_F3);
        LOGENUMVALUE(WXK_F4);
        LOGENUMVALUE(WXK_F5);
        LOGENUMVALUE(WXK_F6);
        LOGENUMVALUE(WXK_F7);
        LOGENUMVALUE(WXK_F8);
        LOGENUMVALUE(WXK_F9);
        LOGENUMVALUE(WXK_F10);
        LOGENUMVALUE(WXK_F11);
        LOGENUMVALUE(WXK_F12);
        LOGENUMVALUE(WXK_F13);
        LOGENUMVALUE(WXK_F14);
        LOGENUMVALUE(WXK_F15);
        LOGENUMVALUE(WXK_F16);
        LOGENUMVALUE(WXK_F17);
        LOGENUMVALUE(WXK_F18);
        LOGENUMVALUE(WXK_F19);
        LOGENUMVALUE(WXK_F20);
        LOGENUMVALUE(WXK_F21);
        LOGENUMVALUE(WXK_F22);
        LOGENUMVALUE(WXK_F23);
        LOGENUMVALUE(WXK_F24);
        LOGENUMVALUE(WXK_NUMLOCK);
        LOGENUMVALUE(WXK_SCROLL);
        LOGENUMVALUE(WXK_PAGEUP);
        LOGENUMVALUE(WXK_PAGEDOWN);
        LOGENUMVALUE(WXK_NUMPAD_SPACE);
        LOGENUMVALUE(WXK_NUMPAD_TAB);
        LOGENUMVALUE(WXK_NUMPAD_ENTER);
        LOGENUMVALUE(WXK_NUMPAD_F1);
        LOGENUMVALUE(WXK_NUMPAD_F2);
        LOGENUMVALUE(WXK_NUMPAD_F3);
        LOGENUMVALUE(WXK_NUMPAD_F4);
        LOGENUMVALUE(WXK_NUMPAD_HOME);
        LOGENUMVALUE(WXK_NUMPAD_LEFT);
        LOGENUMVALUE(WXK_NUMPAD_UP);
        LOGENUMVALUE(WXK_NUMPAD_RIGHT);
        LOGENUMVALUE(WXK_NUMPAD_DOWN);
        LOGENUMVALUE(WXK_NUMPAD_PAGEUP);
        LOGENUMVALUE(WXK_NUMPAD_PAGEDOWN);
        LOGENUMVALUE(WXK_NUMPAD_END);
        LOGENUMVALUE(WXK_NUMPAD_BEGIN);
        LOGENUMVALUE(WXK_NUMPAD_INSERT);
        LOGENUMVALUE(WXK_NUMPAD_DELETE);
        LOGENUMVALUE(WXK_NUMPAD_EQUAL);
        LOGENUMVALUE(WXK_NUMPAD_MULTIPLY);
        LOGENUMVALUE(WXK_NUMPAD_ADD);
        LOGENUMVALUE(WXK_NUMPAD_SEPARATOR);
        LOGENUMVALUE(WXK_NUMPAD_SUBTRACT);
        LOGENUMVALUE(WXK_NUMPAD_DECIMAL);
        LOGENUMVALUE(WXK_NUMPAD_DIVIDE);
        LOGENUMVALUE(WXK_WINDOWS_LEFT);
        LOGENUMVALUE(WXK_WINDOWS_RIGHT);
        LOGENUMVALUE(WXK_WINDOWS_MENU);
        LOGENUMVALUE(WXK_SPECIAL1);
        LOGENUMVALUE(WXK_SPECIAL2);
        LOGENUMVALUE(WXK_SPECIAL3);
        LOGENUMVALUE(WXK_SPECIAL4);
        LOGENUMVALUE(WXK_SPECIAL5);
        LOGENUMVALUE(WXK_SPECIAL6);
        LOGENUMVALUE(WXK_SPECIAL7);
        LOGENUMVALUE(WXK_SPECIAL8);
        LOGENUMVALUE(WXK_SPECIAL9);
        LOGENUMVALUE(WXK_SPECIAL10);
        LOGENUMVALUE(WXK_SPECIAL11);
        LOGENUMVALUE(WXK_SPECIAL12);
        LOGENUMVALUE(WXK_SPECIAL13);
        LOGENUMVALUE(WXK_SPECIAL14);
        LOGENUMVALUE(WXK_SPECIAL15);
        LOGENUMVALUE(WXK_SPECIAL16);
        LOGENUMVALUE(WXK_SPECIAL17);
        LOGENUMVALUE(WXK_SPECIAL18);
        LOGENUMVALUE(WXK_SPECIAL19);
        LOGENUMVALUE(WXK_SPECIAL20);
    default:
        os << "Unknown wxKeyCode (" << static_cast<long>(obj) << ")";
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, const wxKeyboardState& obj)
{
    os  << (obj.ControlDown()   ? 'C' : '_' )
        << (obj.AltDown()       ? 'A' : '_' )
        << (obj.ShiftDown()     ? 'S' : '_' )
        << (obj.MetaDown()      ? 'M' : '_' )
        << (obj.CmdDown()       ? 'C' : '_' );
    return os;
}

std::ostream& operator<<(std::ostream& os, const wxRegion& obj)
{
    wxRegionIterator it(obj);
    os << '{';
    while (it)
    {
        os << '(' << it.GetX() << ',' << it.GetY() << ',' << it.GetW() << ',' << it.GetH() << ')';
        it++;
    }
    os << '}';
    return os;
}

std::ostream& operator<<(std::ostream& os, const wxCompositionMode& obj)
{
    switch (obj)
    {
        LOGENUMVALUE(wxCOMPOSITION_INVALID);
        LOGENUMVALUE(wxCOMPOSITION_CLEAR);
        LOGENUMVALUE(wxCOMPOSITION_SOURCE);
        LOGENUMVALUE(wxCOMPOSITION_OVER);
        LOGENUMVALUE(wxCOMPOSITION_IN);
        LOGENUMVALUE(wxCOMPOSITION_OUT);
        LOGENUMVALUE(wxCOMPOSITION_ATOP);
        LOGENUMVALUE(wxCOMPOSITION_DEST);
        LOGENUMVALUE(wxCOMPOSITION_DEST_OVER);
        LOGENUMVALUE(wxCOMPOSITION_DEST_IN);
        LOGENUMVALUE(wxCOMPOSITION_DEST_OUT);
        LOGENUMVALUE(wxCOMPOSITION_DEST_ATOP);
        LOGENUMVALUE(wxCOMPOSITION_XOR);
        LOGENUMVALUE(wxCOMPOSITION_ADD);
    default:
        os << "Unknown wxCompositionMode (" << static_cast<long>(obj) << ")";

    }
    return os;
}

std::ostream& operator<<(std::ostream& os, const wxInterpolationQuality& obj)
{
    switch (obj)
    {
        LOGENUMVALUE(wxINTERPOLATION_DEFAULT);
        LOGENUMVALUE(wxINTERPOLATION_NONE);
        LOGENUMVALUE(wxINTERPOLATION_FAST);
        LOGENUMVALUE(wxINTERPOLATION_GOOD);
        LOGENUMVALUE(wxINTERPOLATION_BEST);
    default:
        os << "Unknown wxInterpolationQuality (" << static_cast<long>(obj) << ")";

    }
    return os;
}

std::ostream& operator<<(std::ostream& os, const wxAntialiasMode& obj)
{
    switch (obj)
    {
        LOGENUMVALUE(wxANTIALIAS_NONE);
        LOGENUMVALUE(wxANTIALIAS_DEFAULT);
    default:
        os << "Unknown wxAntialiasMode (" << static_cast<long>(obj) << ")";

    }
    return os;
}

std::ostream& operator<<(std::ostream& os, const wxGraphicsContext& obj)
{
    os  << obj.GetCompositionMode()         << '|'
        << obj.GetInterpolationQuality()    << '|'
        << obj.GetAntialiasMode()           ;
    return os;
}

std::ostream& operator<<(std::ostream& os, const wxArrayString& obj)
{
    os << '{';
    for (auto s : obj)
    {
        os << s << ' ';
    }
    os  << '}';
    return os;
}

std::ostream& operator<<(std::ostream& os, const wxDateTime& obj)
{
    if (obj.IsValid())
    {
        os << obj.FormatISOCombined() << '.' << std::setw(3) << std::setfill('0') << obj.GetMillisecond();
    }
    else
    {
        os << "xxxx-xx-xxTxx:xx:xx";
    }
    return os;
}
