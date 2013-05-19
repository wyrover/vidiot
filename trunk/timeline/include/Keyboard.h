#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "Part.h"

namespace gui { namespace timeline {

class Keyboard
    :   public Part
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION METHODS
    //////////////////////////////////////////////////////////////////////////

    Keyboard(Timeline* timeline);
    virtual ~Keyboard();

    //////////////////////////////////////////////////////////////////////////
    // GUI EVENTS
    //////////////////////////////////////////////////////////////////////////

    void update(const wxKeyboardState& state);

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //// //////////////////////////////////////////////////////////////////////

    void setCtrlDown(bool down);
    bool getCtrlDown() const;

    void setShiftDown(bool down);
    bool getShiftDown() const;

    void setAltDown(bool down);
    bool getAltDown() const;

private:

    bool mCtrlDown;
    bool mShiftDown;
    bool mAltDown;
};

}} // namespace

#endif // KEYBOARD_H