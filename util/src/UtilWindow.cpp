// Copyright 2014 Eric Raijmakers.
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

#include "UtilWindow.h"

#include "UtilLog.h"

namespace util { namespace window {

void setIcons(wxTopLevelWindow* window)
{
    wxIconBundle icons;
    icons.AddIcon(util::path::toFileInInstallationDirectory("images","movie_all.ico")); // Icon in title bar of window
    window->SetIcons(icons);
}

wxIcon getIcon(wxString name)
{
    return wxIcon(util::path::toFileInInstallationDirectory("images", name), wxBITMAP_TYPE_XPM);
}

wxBitmap getBitmap(wxString name)
{
    wxBitmapType type(wxBITMAP_TYPE_XPM);

    if (name.EndsWith(".ico"))
    {
        type = wxBITMAP_TYPE_ICO;
    }
    else if (name.EndsWith(".png"))
    {
        type = wxBITMAP_TYPE_PNG;
    }
    else
    {
        ASSERT(name.EndsWith(".xpm"));
    }
    return wxBitmap(util::path::toFileInInstallationDirectory("images", name), type);
}

wxImage getImage(wxString name)
{
    return getBitmap(name).ConvertToImage();
}

}}