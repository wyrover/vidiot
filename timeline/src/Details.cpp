// Copyright 2013,2014 Eric Raijmakers.
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

#include "Details.h"

#include "DetailsClip.h"
#include "DetailsTrim.h"
#include "IClip.h"
#include "Layout.h"
#include "Sequence.h"
#include "Trim.h"
#include "UtilLog.h"
#include "UtilLogWxwidgets.h"
#include "VideoClip.h"
#include "Window.h"

namespace gui { namespace timeline {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

Details::Details(wxWindow* parent, Timeline* timeline)
    :   wxPanel(parent,wxID_ANY,wxDefaultPosition, wxSize(2000,-1))
    ,   mDetails()
{
    LOG_INFO;

    // The order in this list is the order of priority in case two panels want to be shown
    mDetails.push_back(new DetailsTrim(this,*timeline));
    mDetails.push_back(new DetailsClip(this,*timeline));
//        mDetails = (boost::assign::list_of<DetailsPanel*>
//        (static_cast<DetailsPanel*>(new DetailsTrim(this,*timeline)))
//        (static_cast<DetailsPanel*>(new DetailsClip(this,*timeline))))).operator std::list<int>();

    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    mHeader = new wxStaticText(this,wxID_ANY,"", wxDefaultPosition, wxSize(2000,-1), wxBORDER_THEME | wxST_ELLIPSIZE_MIDDLE | wxALIGN_CENTRE);
    mHeader->SetBackgroundColour(Layout::get().DetailsViewHeaderColour);
    sizer->Add(mHeader, wxSizerFlags(0).Center());
    for ( DetailsPanel* details : mDetails )
    {
        sizer->Add(details, wxSizerFlags(1).Expand() );
    }
    SetSizer(sizer);
    update();
}

Details::~Details()
{
    mDetails.clear();
}

//////////////////////////////////////////////////////////////////////////
// TEST
//////////////////////////////////////////////////////////////////////////

wxWindow* Details::getCurrent() const
{
    return mCurrent;
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void Details::update()
{
    bool shown = false;
    for (DetailsPanel* details : mDetails)
    {
        if (details->requestsToBeShown() && !shown)
        {
            if (mHeader->GetLabel() != details->getTitle())
            {
                mHeader->SetLabel(details->getTitle());
            }
            if (mCurrent == details)
            {
                return; // Do not change a thing. Avoid flicker.
            }
            mCurrent = details;
            GetSizer()->Show(details);
            shown = true;
        }
        else
        {
            GetSizer()->Hide(details);
        }
    }
    if (!shown)
    {
        mCurrent = 0;
        mHeader->SetLabel("");
    }
    gui::Window::get().triggerLayout();
}

}} // namespace
