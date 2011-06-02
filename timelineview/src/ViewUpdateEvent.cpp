#include "ViewUpdateEvent.h"

#include "View.h"
#include "UtilLogWxwidgets.h"

namespace gui { namespace timeline {

ViewUpdate::ViewUpdate(View& view, wxRegion area)
    :   mView(&view)
    ,   mArea(area)
{
}

ViewUpdate::~ViewUpdate()
{
}

ViewUpdate::ViewUpdate(const ViewUpdate& other)
    :   mView(other.mView)
    ,   mArea(other.mArea)
{
}

View& ViewUpdate::getView()
{
    return *mView;
}

wxRegion ViewUpdate::getArea()
{
    return mArea;
}

std::ostream& operator<<( std::ostream& os, const ViewUpdate& obj )
{
    os << obj.mView << '|' << obj.mArea;
    return os;
}

DEFINE_EVENT(VIEW_UPDATE_EVENT, ViewUpdateEvent, ViewUpdate);

}} // namespace
