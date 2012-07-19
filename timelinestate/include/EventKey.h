#ifndef TIMELINE_EVENT_KEY_H
#define TIMELINE_EVENT_KEY_H

#include "UtilLogWxwidgets.h"
#include "Part.h"

namespace gui { namespace timeline { namespace state {

template< class MostDerived >
struct EvKey : boost::statechart::event< MostDerived >
{
    EvKey(wxKeyEvent& wxevt, wxPoint pos)
        :   mPosition(pos)
        ,   mWxEvent(wxevt)
    {
    };
    const wxPoint mPosition;
    const wxKeyEvent& mWxEvent;
};

template< class MostDerived >
std::ostream& operator<< (std::ostream& os, const EvKey< MostDerived >& obj)
{
    os  << typeid(obj).name()   << '|' // This typeid is required to distinguish the various 'react' methods
        << obj.mPosition        << '|'
        << obj.mWxEvent;
    return os;
}

struct EvKeyDown : EvKey<EvKeyDown> { EvKeyDown (wxKeyEvent& wxevt, wxPoint pos) : EvKey<EvKeyDown> (wxevt, pos) {} };
struct EvKeyUp   : EvKey<EvKeyUp>   { EvKeyUp   (wxKeyEvent& wxevt, wxPoint pos) : EvKey<EvKeyUp>   (wxevt, pos) {} };

}}} // namespace

#endif // TIMELINE_EVENT_KEY_H