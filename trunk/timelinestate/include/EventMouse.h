#ifndef TIMELINE_EVENT_MOUSE_H
#define TIMELINE_EVENT_MOUSE_H

namespace gui { namespace timeline { namespace state {

template< class MostDerived >
struct EvMouse : boost::statechart::event< MostDerived >
{
    friend std::ostream& operator<< (std::ostream& os, const EvMouse& obj)
    {
        os  << typeid(obj).name(); // This typeid is required to distinguish the various 'react' methods
        return os;
    }
};

struct EvMotion         : EvMouse< EvMotion >          {};
struct EvLeftDown       : EvMouse< EvLeftDown >        {};
struct EvLeftUp         : EvMouse< EvLeftUp >          {};
struct EvLeftDouble     : EvMouse< EvLeftDouble >      {};
struct EvMiddleDown     : EvMouse< EvMiddleDown >      {};
struct EvMiddleUp       : EvMouse< EvMiddleUp >        {};
struct EvMiddleDouble   : EvMouse< EvMiddleDouble >    {};
struct EvRightDown      : EvMouse< EvRightDown >       {};
struct EvRightUp        : EvMouse< EvRightUp >         {};
struct EvRightDouble    : EvMouse< EvRightDouble >     {};
struct EvEnter          : EvMouse< EvEnter >           {};
struct EvLeave          : EvMouse< EvLeave >           {};
struct EvWheel          : EvMouse< EvWheel >           {};

}}} // namespace

#endif // TIMELINE_EVENT_MOUSE_H