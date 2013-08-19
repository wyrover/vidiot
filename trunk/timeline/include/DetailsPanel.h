// Copyright 2013 Eric Raijmakers.
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

#ifndef DETAILS_PANEL_H
#define DETAILS_PANEL_H

#include "Part.h"

namespace gui { namespace timeline {

class DetailsPanel
:   public wxPanel
,   public Part
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    DetailsPanel(wxWindow* parent, Timeline& timeline);
    virtual ~DetailsPanel();

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    bool requestsToBeShown() const;
    wxString getTitle() const;

protected:

    //////////////////////////////////////////////////////////////////////////
    // INTERFACE TOWARDS ACTUAL DETAILS VIEWS
    //////////////////////////////////////////////////////////////////////////

    void requestShow(bool show, wxString title = "");

    void addBox(const wxString& name);

    void showBox(const wxString& name, bool show = true);

    void addOption(const wxString& name, wxWindow* widget);

    /// Show/hide an option and its title
    /// \param widget option that must be shown/hidden
    /// \param show if true then show, otherwise hide
    void showOption(wxWindow* widget, bool show = true);

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    bool mShow;
    wxString mTitle;

    wxBoxSizer*      mTopSizer;  ///< sizer for panel
    wxSizer*         mBoxSizer;  ///< sizer for current box

    std::map<wxString, wxSizer*> mBoxes;

    std::map<wxWindow*, wxSizer*> mMapWindowToSizer;
    std::map<wxWindow*, wxStaticText*> mMapWindowToTitle;

};

}} // namespace

#endif // DETAILS_PANEL_H