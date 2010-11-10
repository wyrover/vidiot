#include "GuiTimeLineDropTarget.h"

#include <boost/foreach.hpp>
#include "GuiTimeLine.h"
#include "GuiTimeLineZoom.h"

namespace gui { namespace timeline {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

GuiTimeLineDropTarget::GuiTimeLineDropTarget(const GuiTimeLineZoom& zoom, GuiTimeLine* timeline)
:   wxDropTarget(new GuiDataObject())
,   mZoom(zoom)
,   mTimeLine(timeline)
{
}
GuiTimeLineDropTarget::~GuiTimeLineDropTarget() 
{
}

//////////////////////////////////////////////////////////////////////////
// ACCESSORS
//////////////////////////////////////////////////////////////////////////

bool GuiTimeLineDropTarget::isDragging() const
{
    return true;
}

//////////////////////////////////////////////////////////////////////////
// FROM WXDROPTARGET
//////////////////////////////////////////////////////////////////////////

//This method may only be called from within OnData().
bool GuiTimeLineDropTarget::GetData()
{
    return true;
}

//Called after OnDrop() returns true.
wxDragResult GuiTimeLineDropTarget::OnData (wxCoord x, wxCoord y, wxDragResult def)
{
    return def;
}

//Called when the mouse is being dragged over the drop target.
wxDragResult GuiTimeLineDropTarget::OnDragOver (wxCoord x, wxCoord y, wxDragResult def)
{
//    mDragShape->SetPosition(wxPoint(x,y));
    mTimeLine->Refresh(); /** /todo use rectangle */
    mTimeLine->Update();
//    mTimeLine->Update();
    return def;

}

//Called when the user drops a data object on the target.
bool GuiTimeLineDropTarget::OnDrop (wxCoord x, wxCoord y)
{
    return true;
}

//Called when the mouse enters the drop target.
wxDragResult GuiTimeLineDropTarget::OnEnter (wxCoord x, wxCoord y, wxDragResult def)
{
    unsigned int w = 200;
    unsigned int h = 10;
    BOOST_FOREACH( model::ProjectViewPtr newChild, dynamic_cast<GuiDataObject*>(m_dataObject)->getAssets())
    {
        w += mZoom.timeToPixels(10000);//newChild->getLength());
    }
  /*  mDragShape = new GuiTimeLineShape(w,h);
    mDragShape->SetShow(true);*/

    return def;
}
//Called when the mouse leaves the drop target.
void GuiTimeLineDropTarget::OnLeave ()
{
    //delete mDragShape;
    //mDragShape = 0;
    mTimeLine->Refresh(); /** /todo use rectangle */
    mTimeLine->Update();
}

}} // namespace
