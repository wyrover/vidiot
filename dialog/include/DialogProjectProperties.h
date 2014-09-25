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

#ifndef DIALOG_PROJECT_PROPERTIES_H
#define DIALOG_PROJECT_PROPERTIES_H

namespace gui {

class DialogProjectProperties
    :   public wxDialog
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    explicit DialogProjectProperties(wxWindow* parent);
    virtual ~DialogProjectProperties();

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    wxRadioBox*             mVideoFrameRate;
    wxSpinCtrl*             mVideoWidth;
    wxSpinCtrl*             mVideoHeight;

    wxComboBox*             mAudioSampleRate;
    wxComboBox*             mAudioNumberOfChannels;

    wxBoxSizer*             mBoxSizer;  ///< box:Sizer for current box

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    void addbox(const wxString& name);
    void addoption(const wxString& name, wxWindow* widget);
};

} // namespace

#endif