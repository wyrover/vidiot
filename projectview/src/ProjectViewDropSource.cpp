// Copyright 2013-2016 Eric Raijmakers.
//
// This file is part of Vidiot.
//
// Vidiot is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Vidiot is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Vidiot. If not, see <http://www.gnu.org/licenses/>.

#include "ProjectViewDropSource.h"
#include "ProjectViewModel.h"
#include "Window.h"
#include "ProjectViewDataObject.h"
#include "Node.h"

namespace gui {

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
    void onPaint( wxPaintEvent &event )
    {
        wxPaintDC dc(this);
        // Don't use DrawBitmap since this gives wrong output when using wxGTK.
        wxMemoryDC dcBmp(mBitmap);
        dc.Blit(wxPoint(0,0), mBitmap.GetSize(), &dcBmp, wxPoint(0,0));
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
    ,   mActive(false)
{
}

ProjectViewDropSource::~ProjectViewDropSource()
{
    if (mHint)
    {
        delete mHint;
        mHint = 0;
    }
}

//////////////////////////////////////////////////////////////////////////
// FROM WXDROPSOURCE
//////////////////////////////////////////////////////////////////////////

bool ProjectViewDropSource::GiveFeedback(wxDragResult effect)
{
    VAR_DEBUG(this)(wxGetMouseState().ControlDown())(wxGetMouseState().ShiftDown());
    mActive = true;
    if (!mFeedback) return wxDropSource::GiveFeedback(effect);

    wxPoint pos = wxGetMousePosition();

    if (!mHint)
    {
        model::NodePtrs assets = getData().getNodes();

        int width = 0;
        int height = 0;
        for (model::NodePtr asset : assets)
        {
            wxRect rect(mCtrl.GetItemRect(wxDataViewItem(asset->id()), mCtrl.GetColumn(0)));
            if (rect.width > width)
            {
                width = rect.width;
            }
            height += rect.height;
        }

        if (width < 20) { width = 20; }
        if (height < 20) { height = 20; }
        wxBitmap bitmap(width, height);
        wxMemoryDC dc(bitmap);

        dc.SetPen(wxPen{ wxColour{ 0, 0, 0 } });
        dc.SetBrush(wxBrush{ wxColour{ 0, 0, 0 }, wxBRUSHSTYLE_TRANSPARENT });
        dc.SetFont(wxFont(wxSize(0, 12), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
        dc.SetTextForeground(*wxWHITE);
        //dc->SetTextBackground(*wxBLUE);

        int y = 0;
        for (model::NodePtr asset : assets)
        {
            wxRect itemRect(mCtrl.GetItemRect(wxDataViewItem(asset->id()), mCtrl.GetColumn(0)));
            drawAsset(&dc, wxRect(0, y, width, itemRect.GetHeight()), asset);
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
    return wxDropSource::GiveFeedback(effect);
}

//////////////////////////////////////////////////////////////////////////
// DRAGGING
//////////////////////////////////////////////////////////////////////////

void ProjectViewDropSource::startDrag(ProjectViewDataObject& data)
{
    SetData(data);
    wxDataObject* pObject = &data;
    VAR_INFO(pObject);
    wxDataFormat format = data.GetFormat();
    wxString id = format.GetId();
    DoDragDrop(wxDrag_DefaultMove);
    mActive = false;
    if (mHint)
    {
        delete mHint;
        mHint = 0;
    }
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

ProjectViewDataObject& ProjectViewDropSource::getData()
{
    return *(dynamic_cast<ProjectViewDataObject*>(GetDataObject()));
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
// TEST
//////////////////////////////////////////////////////////////////////////

bool ProjectViewDropSource::isDragActive() const
{
    return mActive;
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void ProjectViewDropSource::drawAsset(wxDC* dc, wxRect rect, model::NodePtr asset)
{
    int indent = 0;
    model::NodePtr parent = asset->getParent();
    while (parent)
    {
        indent++;
        parent = parent->getParent();
    }
    rect.x += (indent - 1) * 10;

    const wxIcon& icon = mModel.getIcon(asset);
    if (icon.IsOk())
    {
        dc->DrawIcon(icon, rect.x, rect.y + (rect.height - icon.GetHeight())/2);
        rect.x += icon.GetWidth() + 4;
        rect.width -= icon.GetWidth() + 4;
    }

    wxString ellipsizedText = wxControl::Ellipsize(asset->getName(), *dc, wxELLIPSIZE_MIDDLE, rect.width, wxELLIPSIZE_FLAGS_NONE);
    dc->DrawLabel(ellipsizedText, rect, wxALIGN_LEFT);
}

} // namespace
