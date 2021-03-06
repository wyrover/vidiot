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

#pragma once

#include "Enums.h"
#include "UtilEnumSelector.h"
#include "UtilInitAvcodec.h"

namespace gui {

class DialogOptions
    :   public wxPropertySheetDialog
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    DialogOptions(wxWindow* parent);
    virtual ~DialogOptions();

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    wxCheckBox*             mLoadLast;
    wxCheckBox*             mBackupBeforeSave;
    wxSpinCtrl*             mBackupBeforeSaveMaximum;
    wxCheckBox*             mSaveAbsolute;

    wxSpinCtrl*             mMakeSequenceEmptyLength;
    wxSpinCtrl*             mMakeSequencePrefixLength;

    wxRadioBox*             mFrameRate;
    wxSpinCtrl*             mDefaultVideoWidth;
    wxSpinCtrl*             mDefaultVideoHeight;
    EnumSelector<model::VideoScaling>* mDefaultVideoScaling;
    EnumSelector<model::VideoAlignment>* mDefaultVideoAlignment;

    wxComboBox*             mDefaultStillImageLength;

    wxComboBox*             mDefaultAudioSampleRate;
    wxComboBox*             mDefaultAudioNumberOfChannels;

    wxSpinCtrlDouble*       mMarkerBeginAddition;
    wxSpinCtrlDouble*       mMarkerEndAddition;
    wxTextCtrl*             mStrip;
    wxCheckBox*             mTimelineEnableAutoAddTracks;

    wxListBox*              mLanguage;

    EnumSelector<LogLevel>* mSelectLogLevel;
    EnumSelector<LogLevelAvcodec>* mSelectLogLevelAvcodec;
    wxChoice*               mLogLevelAvcodec;
    wxCheckBox*             mShowDebugInfoOnWidgets;
    wxCheckBox*             mLogSequenceOnEdit;

    wxPanel*                mPanel;     ///< tab:The topmost widget
    wxBoxSizer*             mTopSizer;  ///< tab:Sizer for panel
    wxBoxSizer*             mBoxSizer;  ///< box:Sizer for current box

    wxImageList             mIcons;

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    void addtab(const wxString& name);
    void addbox(const wxString& name);
    void addoption(const wxString& name, wxWindow* widget);
    void addnote(const wxString& text);
};

} // namespace
