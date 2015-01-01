// Copyright 2014-2015 Eric Raijmakers.
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

#ifndef EDIT_PROJECT_PROPERTIES_H
#define EDIT_PROJECT_PROPERTIES_H

namespace gui {

class EditProjectProperties
    :   public wxPanel
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    explicit EditProjectProperties(wxWindow* parent);
    virtual ~EditProjectProperties();

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    /// Reads the values from the project and initializes the widgets to the
    /// proper values.
    void read();

    /// Applies the currently selected values to the project ('ok').
    void write();

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    wxRadioBox*             mVideoFrameRate;
    wxSpinCtrl*             mVideoWidth;
    wxSpinCtrl*             mVideoHeight;

    wxComboBox*             mAudioSampleRate;
    wxComboBox*             mAudioNumberOfChannels;

    wxStaticText*           mNote;

    wxBoxSizer*             mBoxSizer;  ///< box:Sizer for current box

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    void addbox(const wxString& name);
    void addoption(const wxString& name, wxWindow* widget);
};

} // namespace

#endif