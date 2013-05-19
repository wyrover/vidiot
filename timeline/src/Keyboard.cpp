#include "Keyboard.h"

#include "UtilLog.h"

namespace gui { namespace timeline {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

Keyboard::Keyboard(Timeline* timeline)
:   Part(timeline)
,   mCtrlDown(false)
,   mShiftDown(false)
,   mAltDown(false)
{
    VAR_DEBUG(this);
}

Keyboard::~Keyboard()
{
    VAR_DEBUG(this);
}

//////////////////////////////////////////////////////////////////////////
// GUI EVENTS
//////////////////////////////////////////////////////////////////////////

void Keyboard::update(const wxKeyboardState& state)
{
    bool CtrlDown(state.ControlDown());
    bool ShiftDown(state.ShiftDown());
    bool AltDown(state.AltDown());
    VAR_DEBUG(CtrlDown)(ShiftDown)(AltDown);
    setCtrlDown(CtrlDown);
    setShiftDown(ShiftDown);
    setAltDown(AltDown);
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

void Keyboard::setCtrlDown(bool down)
{
    mCtrlDown = down;
}

bool Keyboard::getCtrlDown() const
{
    return mCtrlDown;
}

void Keyboard::setShiftDown(bool down)
{
    mShiftDown = down;
}

bool Keyboard::getShiftDown() const
{
    return mShiftDown;
}

void Keyboard::setAltDown(bool down)
{
    mAltDown = down;
}

bool Keyboard::getAltDown() const
{
    return mAltDown;
}
}} // namespace