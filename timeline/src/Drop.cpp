#include "Drop.h"

#include <wx/dc.h>
#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>
#include "AudioClip.h"
#include "AudioFile.h"
#include "ClipView.h"
#include "File.h"
#include "GuiDataObject.h"
#include "SequenceView.h"
#include "Layout.h"
#include "MousePointer.h"
#include "PositionInfo.h"
#include "Timeline.h"
#include "TrackView.h"
#include "UtilLog.h"
#include "GuiProjectView.h"
#include "VideoClip.h"
#include "VideoFile.h"
#include "ViewMap.h"
#include "Zoom.h"

namespace gui { namespace timeline {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

Drop::Drop(Timeline* timeline)
:   wxDropTarget(new GuiDataObject())
,   Part(timeline)
{
    //getTimeline().SetDropTarget(this);// Uncommented, since the destruction will then be done by wxwidgets
    VAR_DEBUG(this);
}

Drop::~Drop() 
{
    VAR_DEBUG(this);
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
  /*  getTimeline().Refresh(); /** /todo use rectangle */
   /* getTimeline().Update();*/
//    mTimeLine->Update();
    mDropArea.SetLeft(x);
    mDropArea.SetTop(y);
    getSequenceView().invalidateBitmap();

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
    std::list<model::IControlPtr> draggedAssets = GuiProjectView::current()->getDraggedAssets();
    unsigned int w = 0;
    unsigned int h = 10;
    BOOST_FOREACH( model::IControlPtr asset, draggedAssets )
    {
        model::FilePtr file = boost::dynamic_pointer_cast<model::File>(asset);
        if (file)
        {
            VAR_DEBUG(file);
            model::VideoFilePtr videoFile = boost::make_shared<model::VideoFile>(file->getPath());
            model::AudioFilePtr audioFile = boost::make_shared<model::AudioFile>(file->getPath());
            model::VideoClipPtr videoClip = boost::make_shared<model::VideoClip>(videoFile);
            model::AudioClipPtr audioClip = boost::make_shared<model::AudioClip>(audioFile);
            videoClip->setLink(audioClip);
            audioClip->setLink(videoClip);
            w += getZoom().ptsToPixels(file->getLength());
        }
    }
    mDropArea = wxRect(x,y,w,h);
    getSequenceView().invalidateBitmap();
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

//////////////////////////////////////////////////////////////////////////
// DRAWING
//////////////////////////////////////////////////////////////////////////

void Drop::draw(wxDC& dc) const
{
    if (!mDropArea.IsEmpty())
    {
        dc.SetPen(Layout::sDropAreaPen);
        dc.SetBrush(Layout::sDropAreaBrush);
        dc.DrawRectangle(mDropArea);
    }
}

}} // namespace
