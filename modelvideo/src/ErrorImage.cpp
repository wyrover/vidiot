// Copyright 2014-2016 Eric Raijmakers.
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

#include "ErrorImage.h"

#include "Properties.h"

namespace model {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

wxImagePtr getErrorImage(const wxFileName& filename)
{
    wxImagePtr image = boost::make_shared<wxImage>(model::Properties::get().getVideoSize());
    wxGraphicsContext* gc = wxGraphicsContext::Create(*image);
    wxString error_message1 = _("Missing file: ");
    wxString error_message2 = filename.GetFullName();
    wxFont errorFont = wxFont(wxSize(5, 20), wxFONTFAMILY_DECORATIVE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD);
    gc->SetFont(errorFont, wxColour(255, 255, 255));
    wxDouble w, h, d, e;
    gc->GetTextExtent(error_message1, &w, &h, &d, &e);
    gc->DrawText(error_message1, 5, 5);
    gc->DrawText(error_message2, 5, 5 + h);
    delete gc;
    return image;
}

} // namespace
