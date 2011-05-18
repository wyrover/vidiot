#ifndef TIMELINE_EVENT_DRAG_H
#define TIMELINE_EVENT_DRAG_H

#include <boost/statechart/event.hpp>
#include "UtilLogWxwidgets.h"

namespace gui { namespace timeline { namespace state {

template< class MostDerived >
struct EvDrag : boost::statechart::event< MostDerived >
{
    EvDrag(int x, int y)
        :   mPosition(x,y)
    {
    };
    const wxPoint mPosition;
};

template< class MostDerived >
std::ostream& operator<< (std::ostream& os, const EvDrag< MostDerived >& obj)
{
    os  << typeid(obj).name() << '|' // This typeid is required to distinguish the various 'react' methods
        << obj.mPosition;
    return os;
}

struct EvDragEnter  : EvDrag< EvDragEnter > { EvDragEnter(int x, int y) : EvDrag<EvDragEnter>(x,y) {} };
struct EvDragMove   : EvDrag< EvDragMove >  { EvDragMove(int x, int y)  : EvDrag<EvDragMove> (x,y) {} };
struct EvDragDrop   : EvDrag< EvDragDrop >  { EvDragDrop(int x, int y)  : EvDrag<EvDragDrop> (x,y) {} };
struct EvDragEnd    : EvDrag< EvDragEnd >   { EvDragEnd(int x, int y)   : EvDrag<EvDragEnd>  (x,y) {} };

}}} // namespace

#endif // TIMELINE_EVENT_DRAG_H
