#include "Drop.h"

#include <boost/foreach.hpp>
#include "GuiTimeLine.h"
#include "Zoom.h"

namespace gui { namespace timeline {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

Drop::Drop()
:   wxDropTarget(new GuiDataObject())
{
}

void Drop::init()
{
    getTimeline().SetDropTarget(this);
}

Drop::~Drop() 
{
}

//////////////////////////////////////////////////////////////////////////
// ACCESSORS
//////////////////////////////////////////////////////////////////////////

bool Drop::isDragging() const
{
    return true;
}

//////////////////////////////////////////////////////////////////////////
// FROM WXDROPTARGET
//////////////////////////////////////////////////////////////////////////

//This method may only be called from within OnData().
bool Drop::GetData()
{
    return true;
}

//Called after OnDrop() returns true.
wxDragResult Drop::OnData (wxCoord x, wxCoord y, wxDragResult def)
{
    return def;
}

//Called when the mouse is being dragged over the drop target.
wxDragResult Drop::OnDragOver (wxCoord x, wxCoord y, wxDragResult def)
{
//    mDragShape->SetPosition(wxPoint(x,y));
    getTimeline().Refresh(); /** /todo use rectangle */
    getTimeline().Update();
//    mTimeLine->Update();
    return def;

}

//Called when the user drops a data object on the target.
bool Drop::OnDrop (wxCoord x, wxCoord y)
{
    return true;
}

//Called when the mouse enters the drop target.
wxDragResult Drop::OnEnter (wxCoord x, wxCoord y, wxDragResult def)
{
    unsigned int w = 200;
    unsigned int h = 10;
    BOOST_FOREACH( model::ProjectViewPtr newChild, dynamic_cast<GuiDataObject*>(m_dataObject)->getAssets())
    {
        w += getZoom().timeToPixels(10000);//newChild->getLength());
    }
  /*  mDragShape = new GuiTimeLineShape(w,h);
    mDragShape->SetShow(true);*/

    return def;
}
//Called when the mouse leaves the drop target.
void Drop::OnLeave ()
{
    //delete mDragShape;
    //mDragShape = 0;
    getTimeline().Refresh(); /** /todo use rectangle */
    getTimeline().Update();
}

}} // namespace
