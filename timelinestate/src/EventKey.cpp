#include "EventKey.h"

#include "UtilLog.h"
#include "UtilLogWxwidgets.h"

namespace gui { namespace timeline { namespace state {

EvKey::EvKey(
    bool controldown,
    bool shiftdown,
    bool altdown,
    wxChar unicodekey,
    int keycode,
    wxPoint position)
    :   mCtrlDown(controldown)
    ,   mShiftDown(shiftdown)
    ,   mAltDown(altdown)
    ,   mUnicodeKey(unicodekey)
    ,   mKeyCode(static_cast<wxKeyCode>(keycode))
    ,   mPosition(position)
{
};

bool EvKey::getCtrlDown() const
{
    return mCtrlDown;
}

bool EvKey::getShiftDown() const
{
    return mShiftDown;
}

bool EvKey::getAltDown() const
{
    return mAltDown;
}

bool EvKey::hasUnicodeKey() const
{
    return mUnicodeKey != WXK_NONE;
}

wxChar EvKey::getUnicodeKey() const
{
    ASSERT(hasUnicodeKey());
    return mUnicodeKey;
}

int EvKey::getKeyCode() const
{
    return mKeyCode;
}

wxPoint EvKey::getPosition() const
{
    return mPosition;
}

std::ostream& operator<< (std::ostream& os, const EvKey& obj)
{
    os  << typeid(obj).name() << '|' // This typeid is required to distinguish the various 'react' methods
        << obj.mCtrlDown      << '|'
        << obj.mShiftDown     << '|'
        << obj.mAltDown       << '|'
        << obj.mKeyCode       << '|'
        << obj.mUnicodeKey    << '|'
        << obj.mPosition;
    return os;
}

EvKeyDown::EvKeyDown(bool controldown, bool shiftdown, bool altdown, wxChar unicodekey, int keycode, wxPoint position)
    : EvKey(controldown, shiftdown, altdown, unicodekey, keycode, position)
{
    VAR_INFO(*this);
}

EvKeyUp::EvKeyUp(bool controldown, bool shiftdown, bool altdown, wxChar unicodekey, int keycode, wxPoint position)
    : EvKey(controldown, shiftdown, altdown, unicodekey, keycode, position)
{
    VAR_INFO(*this);
}

}}} // namespace