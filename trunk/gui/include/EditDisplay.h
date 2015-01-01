// Copyright 2013-2015 Eric Raijmakers.
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

#ifndef EDIT_DISPLAY_H
#define EDIT_DISPLAY_H

#include "VideoFrame.h"

namespace gui {

class EditDisplay
:   public wxControl
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION METHODS
    //////////////////////////////////////////////////////////////////////////

    EditDisplay(wxWindow *parent);
    virtual ~EditDisplay();

    //////////////////////////////////////////////////////////////////////////
    // SHOW AN EDIT ACTION
    //////////////////////////////////////////////////////////////////////////

    void show(const wxBitmapPtr& bitmap);

    //////////////////////////////////////////////////////////////////////////
    // GET & SET
    //////////////////////////////////////////////////////////////////////////

    wxSize getSize() const;

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    wxBitmapPtr mCurrentBitmap;

    int mWidth;
    int mHeight;

    //////////////////////////////////////////////////////////////////////////
    // GUI METHODS
    //////////////////////////////////////////////////////////////////////////

    void OnSize(wxSizeEvent& event);
    void OnPaint(wxPaintEvent& event);
    void OnEraseBackground(wxEraseEvent& event);
};

} // namespace

#endif