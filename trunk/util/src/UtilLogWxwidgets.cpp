#include "UtilLogWxwidgets.h"

std::ostream& operator<< (std::ostream& os, const wxFileName& obj)
{
    os << obj.GetPath() << '|' << obj.GetName() << '|' << obj.GetExt();
    return os;
}

std::ostream& operator<< (std::ostream& os, const wxPoint& obj)
{
    os << '(' << std::setfill(' ') << std::setw(6) << obj.x << ',' << std::setw(6) << obj.y << ')';
    return os;
}

std::ostream& operator<< (std::ostream& os, const wxSize& obj)
{
    os << '(' << obj.GetWidth() << ',' << obj.GetHeight() << ')';
    return os;
}

std::ostream& operator<< (std::ostream& os, const wxMouseEvent& obj)
{
    os  << obj.GetX() << ','
        << obj.GetY() << ','
        << static_cast<const wxKeyboardState&>(obj);
    return os;
}

std::ostream& operator<< (std::ostream& os, const wxKeyCode& obj)
{
    std::string key;
#define LOGKEY(code) case code: key = #code; break
    switch (obj)
    {
        LOGKEY(WXK_BACK);
        LOGKEY(WXK_TAB);
        LOGKEY(WXK_RETURN);
        LOGKEY(WXK_ESCAPE);
        LOGKEY(WXK_SPACE);
        LOGKEY(WXK_DELETE);
        LOGKEY(WXK_START);
        LOGKEY(WXK_LBUTTON);
        LOGKEY(WXK_RBUTTON);
        LOGKEY(WXK_CANCEL);
        LOGKEY(WXK_MBUTTON);
        LOGKEY(WXK_CLEAR);
        LOGKEY(WXK_SHIFT);
        LOGKEY(WXK_ALT);
        LOGKEY(WXK_CONTROL);
        LOGKEY(WXK_MENU);
        LOGKEY(WXK_PAUSE);
        LOGKEY(WXK_CAPITAL);
        LOGKEY(WXK_END);
        LOGKEY(WXK_HOME);
        LOGKEY(WXK_LEFT);
        LOGKEY(WXK_UP);
        LOGKEY(WXK_RIGHT);
        LOGKEY(WXK_DOWN);
        LOGKEY(WXK_SELECT);
        LOGKEY(WXK_PRINT);
        LOGKEY(WXK_EXECUTE);
        LOGKEY(WXK_SNAPSHOT);
        LOGKEY(WXK_INSERT);
        LOGKEY(WXK_HELP);
        LOGKEY(WXK_NUMPAD0);
        LOGKEY(WXK_NUMPAD1);
        LOGKEY(WXK_NUMPAD2);
        LOGKEY(WXK_NUMPAD3);
        LOGKEY(WXK_NUMPAD4);
        LOGKEY(WXK_NUMPAD5);
        LOGKEY(WXK_NUMPAD6);
        LOGKEY(WXK_NUMPAD7);
        LOGKEY(WXK_NUMPAD8);
        LOGKEY(WXK_NUMPAD9);
        LOGKEY(WXK_MULTIPLY);
        LOGKEY(WXK_ADD);
        LOGKEY(WXK_SEPARATOR);
        LOGKEY(WXK_SUBTRACT);
        LOGKEY(WXK_DECIMAL);
        LOGKEY(WXK_DIVIDE);
        LOGKEY(WXK_F1);
        LOGKEY(WXK_F2);
        LOGKEY(WXK_F3);
        LOGKEY(WXK_F4);
        LOGKEY(WXK_F5);
        LOGKEY(WXK_F6);
        LOGKEY(WXK_F7);
        LOGKEY(WXK_F8);
        LOGKEY(WXK_F9);
        LOGKEY(WXK_F10);
        LOGKEY(WXK_F11);
        LOGKEY(WXK_F12);
        LOGKEY(WXK_F13);
        LOGKEY(WXK_F14);
        LOGKEY(WXK_F15);
        LOGKEY(WXK_F16);
        LOGKEY(WXK_F17);
        LOGKEY(WXK_F18);
        LOGKEY(WXK_F19);
        LOGKEY(WXK_F20);
        LOGKEY(WXK_F21);
        LOGKEY(WXK_F22);
        LOGKEY(WXK_F23);
        LOGKEY(WXK_F24);
        LOGKEY(WXK_NUMLOCK);
        LOGKEY(WXK_SCROLL);
        LOGKEY(WXK_PAGEUP);
        LOGKEY(WXK_PAGEDOWN);
        LOGKEY(WXK_NUMPAD_SPACE);
        LOGKEY(WXK_NUMPAD_TAB);
        LOGKEY(WXK_NUMPAD_ENTER);
        LOGKEY(WXK_NUMPAD_F1);
        LOGKEY(WXK_NUMPAD_F2);
        LOGKEY(WXK_NUMPAD_F3);
        LOGKEY(WXK_NUMPAD_F4);
        LOGKEY(WXK_NUMPAD_HOME);
        LOGKEY(WXK_NUMPAD_LEFT);
        LOGKEY(WXK_NUMPAD_UP);
        LOGKEY(WXK_NUMPAD_RIGHT);
        LOGKEY(WXK_NUMPAD_DOWN);
        LOGKEY(WXK_NUMPAD_PAGEUP);
        LOGKEY(WXK_NUMPAD_PAGEDOWN);
        LOGKEY(WXK_NUMPAD_END);
        LOGKEY(WXK_NUMPAD_BEGIN);
        LOGKEY(WXK_NUMPAD_INSERT);
        LOGKEY(WXK_NUMPAD_DELETE);
        LOGKEY(WXK_NUMPAD_EQUAL);
        LOGKEY(WXK_NUMPAD_MULTIPLY);
        LOGKEY(WXK_NUMPAD_ADD);
        LOGKEY(WXK_NUMPAD_SEPARATOR);
        LOGKEY(WXK_NUMPAD_SUBTRACT);
        LOGKEY(WXK_NUMPAD_DECIMAL);
        LOGKEY(WXK_NUMPAD_DIVIDE);
        LOGKEY(WXK_WINDOWS_LEFT);
        LOGKEY(WXK_WINDOWS_RIGHT);
        LOGKEY(WXK_WINDOWS_MENU);
        LOGKEY(WXK_SPECIAL1);
        LOGKEY(WXK_SPECIAL2);
        LOGKEY(WXK_SPECIAL3);
        LOGKEY(WXK_SPECIAL4);
        LOGKEY(WXK_SPECIAL5);
        LOGKEY(WXK_SPECIAL6);
        LOGKEY(WXK_SPECIAL7);
        LOGKEY(WXK_SPECIAL8);
        LOGKEY(WXK_SPECIAL9);
        LOGKEY(WXK_SPECIAL10);
        LOGKEY(WXK_SPECIAL11);
        LOGKEY(WXK_SPECIAL12);
        LOGKEY(WXK_SPECIAL13);
        LOGKEY(WXK_SPECIAL14);
        LOGKEY(WXK_SPECIAL15);
        LOGKEY(WXK_SPECIAL16);
        LOGKEY(WXK_SPECIAL17);
        LOGKEY(WXK_SPECIAL18);
        LOGKEY(WXK_SPECIAL19);
        LOGKEY(WXK_SPECIAL20);
        default:
            key = obj;
    }
    os  << key;
    return os;
}

std::ostream& operator<< (std::ostream& os, const wxKeyboardState& obj)
{
    os  << (obj.ControlDown()   ? 'C' : '_' )
        << (obj.AltDown()       ? 'A' : '_' )
        << (obj.ShiftDown()     ? 'S' : '_' )
        << (obj.MetaDown()      ? 'M' : '_' )
        << (obj.CmdDown()       ? 'C' : '_' );
    return os;
}

std::ostream& operator<< (std::ostream& os, const wxRegion& obj)
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