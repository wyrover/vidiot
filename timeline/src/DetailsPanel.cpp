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

#include "DetailsPanel.h"

#include "Details.h"

namespace gui { namespace timeline {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

DetailsPanel::DetailsPanel(wxWindow* parent, Timeline& timeline)
    :   wxPanel(parent)
    ,   Part(&timeline)
    ,   mShow(false)
    ,   mTitle("")
    ,   mTopSizer(0)
    ,   mBoxSizer(0)
{
    mTopSizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(mTopSizer);
}

DetailsPanel::~DetailsPanel()
{
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

bool DetailsPanel::requestsToBeShown() const
{
    return mShow;
}

wxString DetailsPanel::getTitle() const
{
    return mTitle;
}

//////////////////////////////////////////////////////////////////////////
// INTERFACE TOWARDS ACTUAL DETAILS VIEWS
//////////////////////////////////////////////////////////////////////////

void DetailsPanel::requestShow(bool show, const wxString& title)
{
    VAR_DEBUG(this)(show)(title);
    mShow = show;
    mTitle = title;
    static_cast<Details*>(GetParent())->update();
}

wxFlexGridSizer* DetailsPanel::addBox(boost::optional<wxString> name, int vgap)
{
    ASSERT(mTopSizer);
    mBoxSizer = new wxFlexGridSizer(2, vgap, 0);
    mBoxSizer->AddGrowableCol(1);
    if (name)
    {
        wxStaticBoxSizer* staticBoxSizer = new wxStaticBoxSizer(wxVERTICAL, this, *name);
        staticBoxSizer->Add(mBoxSizer, wxSizerFlags(0).Expand());
        mTopSizer->Add(staticBoxSizer, wxSizerFlags(0).Expand());
        mBoxes[*name] = staticBoxSizer;
    }
    else
    {
        mTopSizer->Add(mBoxSizer, wxSizerFlags(0).Expand());
    }

    mTopSizer->Layout();
    return mBoxSizer;
}


void DetailsPanel::addWidget(wxWindow* window)
{
    ASSERT(mTopSizer);
    mTopSizer->Add(window, wxSizerFlags(0).Expand());
}

void DetailsPanel::setBox(wxFlexGridSizer* box)
{
    mBoxSizer = box;
}

void DetailsPanel::setBoxTitle(const wxString& boxname, const wxString& title)
{
    ASSERT_MAP_CONTAINS(mBoxes,boxname);
    mBoxes[boxname]->GetStaticBox()->SetLabel(title);
}

void DetailsPanel::show(wxWindow* window, bool show)
{
    mTopSizer->Show(window, show);
}

void DetailsPanel::showBox(const wxString& name, bool show)
{
    ASSERT_MAP_CONTAINS(mBoxes,name);
    mTopSizer->Show(mBoxes[name], show);
}

void DetailsPanel::addOption(const wxString& name, wxWindow* widget)
{
    ASSERT(mBoxSizer);
    wxStaticText* title = new wxStaticText(this, wxID_ANY, name, wxDefaultPosition, wxSize(120,-1), wxST_ELLIPSIZE_END);
    mBoxSizer->Add(title, wxSizerFlags(0).CenterVertical().Left());//, 0, wxALL|wxALIGN_TOP, 0);
    mBoxSizer->Add(widget, wxSizerFlags(1).Expand());

    mMapWindowToSizer[widget] = mBoxSizer;
    mMapWindowToTitle[widget] = title;
}

void DetailsPanel::showOption(wxWindow* widget, bool show)
{
    ASSERT_MAP_CONTAINS(mMapWindowToSizer,widget);
    ASSERT_MAP_CONTAINS(mMapWindowToTitle,widget);
    mMapWindowToSizer[widget]->Show(widget,show);
    mMapWindowToSizer[widget]->Show(mMapWindowToTitle[widget],show);
}

wxWindow* DetailsPanel::getTitle(wxWindow* widget) const
{
    ASSERT_MAP_CONTAINS(mMapWindowToTitle,widget);
    return mMapWindowToTitle.find(widget)->second;
}


}} // namespace