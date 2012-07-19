#ifndef VIEW_UPDATE_H
#define VIEW_UPDATE_H

#include "UtilEvent.h"

namespace gui { namespace timeline {

class View;

class ViewUpdate
{
public:
    ViewUpdate(View& view, wxRegion area);
    ~ViewUpdate();
    ViewUpdate(const ViewUpdate& other);
    View& getView();
    wxRegion getArea();
private:
    View* mView;    // Stored as pointer, not reference for more info during debugging in IDE
    wxRegion mArea;
    friend std::ostream& operator<<( std::ostream& os, const ViewUpdate& obj );
};

DECLARE_EVENT(VIEW_UPDATE_EVENT, ViewUpdateEvent, ViewUpdate);

}} // namespace

#endif // VIEW_UPDATE_H