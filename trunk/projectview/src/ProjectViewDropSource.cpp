#include "ProjectViewDropSource.h"
#include <wx/dcclient.h>
#include <wx/dcmemory.h>
#include <boost/foreach.hpp>
#include "ProjectViewModel.h"
#include "Layout.h"
#include "GuiWindow.h"
#include "GuiDataObject.h"
#include "AProjectViewNode.h"

namespace gui {

ProjectViewDropSource* sCurrent = 0;

struct wxBitmapCanvas
    :   public wxWindow
{
    wxBitmapCanvas(wxWindow *parent, const wxBitmap &bitmap)
        :   wxWindow( parent, wxID_ANY, wxPoint(0,0), bitmap.GetSize() )
        ,   mBitmap(bitmap)
    {
        Bind(wxEVT_PAINT, &wxBitmapCanvas::onPaint, this);
    }
    ~wxBitmapCanvas()
    {
        Unbind(wxEVT_PAINT, &wxBitmapCanvas::onPaint, this);
    }
    void onPaint( wxPaintEvent &WXUNUSED(event) )
    {
        wxPaintDC dc(this);
        dc.DrawBitmap( mBitmap, 0, 0);
    }
    wxBitmap mBitmap;
};

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

ProjectViewDropSource::ProjectViewDropSource(wxDataViewCtrl& ctrl, ProjectViewModel& model)
    :   wxDropSource(&ctrl)
    ,   mCtrl(ctrl)
    ,   mModel(model)
    ,   mHint(0)
    ,   mFeedback(true)
{
    sCurrent = this;
}

ProjectViewDropSource::~ProjectViewDropSource()
{
    if (mHint)
    {
        delete mHint;
        mHint = 0;
    }
    sCurrent = 0;
}

// static 
ProjectViewDropSource& ProjectViewDropSource::current()
{
    return *sCurrent;
}

//////////////////////////////////////////////////////////////////////////
// FROM WXDROPSOURCE
//////////////////////////////////////////////////////////////////////////

bool ProjectViewDropSource::GiveFeedback(wxDragResult effect)
{
    if (!mFeedback) return true;

    wxPoint pos = wxGetMousePosition();

    if (!mHint)
    {
        model::ProjectViewPtrs assets = getData().getAssets();

        int width = 0;
        int height = 0;
        BOOST_FOREACH( model::ProjectViewPtr asset, assets )
        {
            wxRect rect(mCtrl.GetItemRect(wxDataViewItem(asset->id()), mCtrl.GetColumn(0)));
            if (rect.width > width)
            {
                width = rect.width;
            }
            height += rect.height;
        }

        wxBitmap bitmap(width, height);
        wxMemoryDC dc(bitmap);

        dc.SetPen(*wxBLACK_PEN);
        dc.SetBrush(*wxTRANSPARENT_BRUSH);
        dc.SetFont(*Layout::sNormalFont);
        dc.SetTextForeground(*wxWHITE);
        //dc->SetTextBackground(*wxBLUE);

        int y = 0;
        BOOST_FOREACH( model::ProjectViewPtr asset, assets )
        {
            wxRect itemRect(mCtrl.GetItemRect(wxDataViewItem(asset->id()), mCtrl.GetColumn(0)));
            drawAsset(&dc, wxRect(0,y,width,itemRect.GetHeight()), asset);
            y += itemRect.GetHeight();
        }
        dc.SelectObject(wxNullBitmap);

        mHint = new wxFrame(&mCtrl, wxID_ANY, wxEmptyString, pos, bitmap.GetSize(), wxFRAME_TOOL_WINDOW | wxFRAME_FLOAT_ON_PARENT | wxFRAME_NO_TASKBAR | wxNO_BORDER);
        new wxBitmapCanvas(mHint, bitmap);
        mHint->Show();
    }
    else
    {
        mHint->Move( pos.x + 5, pos.y + 5 ); // NOTE: Be sure to have a minimum offset of (1,1) wrt pointer position. Otherwise, the DND handling does not work.
        mHint->SetTransparent( 128 );
    }
    return true;
}

// TODO
//enum  	wxDragResult {
//    wxDragError,
//    wxDragNone,
//    wxDragCopy,
//    wxDragMove,
//    wxDragLink,
//    wxDragCancel
//}
//
//void wxDropSource::SetCursor 	( 	wxDragResult  	res,
//                                 const wxCursor &  	cursor	 
//                                 ) 			
//
//                                 Set the icon to use for a certain drag result.
//
//Parameters:
//res 	The drag result to set the icon for.
//cursor 	The ion to show when this drag result occurs. 

//////////////////////////////////////////////////////////////////////////
// DRAGGING
//////////////////////////////////////////////////////////////////////////

void ProjectViewDropSource::startDrag(GuiDataObject& data)
{
    SetData(data);
    DoDragDrop(wxDrag_DefaultMove);
    if (mHint)
    {
        delete mHint;
        mHint = 0;
    }
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

GuiDataObject& ProjectViewDropSource::getData()
{
    return *(dynamic_cast<GuiDataObject*>(GetDataObject()));
}

void ProjectViewDropSource::setFeedback(bool enabled)
{
    mFeedback = enabled;
    if (mHint)
    {
        if (mFeedback)
        {
            mHint->Show();
        }
        else
        {
            mHint->Hide();
        }
    }

}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void ProjectViewDropSource::drawAsset(wxDC* dc, wxRect rect, model::ProjectViewPtr asset)
{
    int xoffset = 0;

    const wxIcon& icon = mModel.getIcon(asset);

    // todo indent based on indent in tree....
    if (icon.IsOk())
    {
        dc->DrawIcon(icon, rect.x, rect.y + (rect.height - icon.GetHeight())/2);
        xoffset = icon.GetWidth() + 4;
    }

    rect.x += xoffset;
    rect.width -= xoffset;

    wxString ellipsizedText = wxControl::Ellipsize(asset->getName(), *dc, wxELLIPSIZE_MIDDLE, rect.width, wxELLIPSIZE_FLAGS_NONE);
    dc->DrawLabel(ellipsizedText, rect, wxALIGN_LEFT);
}

} // namespace
