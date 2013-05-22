#ifndef TIMELINE_EVENT_DRAG_H
#define TIMELINE_EVENT_DRAG_H

namespace gui { namespace timeline { namespace state {

template< class MostDerived >
struct EvDrag : boost::statechart::event< MostDerived >
{
    friend std::ostream& operator<< (std::ostream& os, const EvDrag& obj)
    {
        os  << typeid(obj).name(); // This typeid is required to distinguish the various 'react' methods
        return os;
    }
};

struct EvDragEnter  : EvDrag< EvDragEnter > {};
struct EvDragMove   : EvDrag< EvDragMove >  {};
struct EvDragDrop   : EvDrag< EvDragDrop >  {};
struct EvDragEnd    : EvDrag< EvDragEnd >   {};

}}} // namespace

#endif // TIMELINE_EVENT_DRAG_H