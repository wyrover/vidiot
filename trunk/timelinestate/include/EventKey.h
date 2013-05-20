#ifndef TIMELINE_EVENT_KEY_H
#define TIMELINE_EVENT_KEY_H

namespace gui { namespace timeline { namespace state {

struct EvKey
{
    EvKey(bool controldown, bool shiftdown, bool altdown, wxChar unicodekey, int keycode, wxPoint position);

    bool getCtrlDown() const;
    bool getShiftDown() const;
    bool getAltDown() const;
    bool hasUnicodeKey() const;
    wxChar getUnicodeKey() const;
    int getKeyCode() const;
    wxPoint getPosition() const;

private:

    const bool mCtrlDown;
    const bool mShiftDown;
    const bool mAltDown;
    const wxKeyCode mKeyCode;
    const wxChar mUnicodeKey;
    const wxPoint mPosition;

    friend std::ostream& operator<< (std::ostream& os, const EvKey& obj);
};

struct EvKeyDown
    : public EvKey
    , public boost::statechart::event< EvKeyDown >
{
    EvKeyDown(bool controldown, bool shiftdown, bool altdown, wxChar unicodekey, int keycode, wxPoint position);
};

struct EvKeyUp
    : public EvKey
    , public boost::statechart::event< EvKeyUp >
{
    EvKeyUp(bool controldown, bool shiftdown, bool altdown, wxChar unicodekey, int keycode, wxPoint position);
};

}}} // namespace

#endif // TIMELINE_EVENT_KEY_H