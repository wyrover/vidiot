#ifndef TIMELINE_EVENT_PART_H
#define TIMELINE_EVENT_PART_H

#include <boost/statechart/event.hpp>
#include "Part.h"
#include "Zoom.h"
#include "Scrolling.h"

namespace gui { namespace timeline { namespace state {

template< class MostDerived, class ORIGINALEVENT >
struct EvPart : boost::statechart::event< MostDerived >
{
    explicit EvPart(ORIGINALEVENT& event)
        :   mEvent(event)
    {
    };
    const ORIGINALEVENT mEvent;
};

template< class MostDerived, class ORIGINALEVENT >
std::ostream& operator<< (std::ostream& os, const EvPart< MostDerived, ORIGINALEVENT >& obj)
{
    os  << typeid(obj).name() << '|' // This typeid is required to distinguish the various 'react' methods
        << obj.mEvent;
    return os;
}

struct EvZoomChanged    : EvPart<EvZoomChanged,     timeline::ZoomChangeEvent>      {    EvZoomChanged(timeline::ZoomChangeEvent& event)        : EvPart<EvZoomChanged, timeline::ZoomChangeEvent>(event) {} };
struct EvScrollChanged  : EvPart<EvScrollChanged,   timeline::ScrollChangeEvent>    {    EvScrollChanged(timeline::ScrollChangeEvent& event)    : EvPart<EvScrollChanged, timeline::ScrollChangeEvent>(event) {} };

}}} // namespace

#endif // TIMELINE_EVENT_PART_H
