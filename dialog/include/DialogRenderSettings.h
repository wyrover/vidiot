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

#ifndef DIALOG_RENDER_SETTINGS_H
#define DIALOG_RENDER_SETTINGS_H

#include "AudioCodecParameter.h"
#include "ICodecParameter.h"
#include "UtilEnumSelector.h"
#include "UtilInt.h"
#include "UtilSingleInstance.h"
#include "VideoCodecParameter.h"

namespace gui {

class DialogRenderSettings
    :   public wxDialog
    ,   public model::render::ICodecParameterChangeListener
    ,   public SingleInstance<DialogRenderSettings>
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    explicit DialogRenderSettings(model::SequencePtr sequence);
    virtual ~DialogRenderSettings();

    //////////////////////////////////////////////////////////////////////////
    // EVENTS
    //////////////////////////////////////////////////////////////////////////

    void onVideoCodecChanged(wxCommandEvent& event);
    void onAudioCodecChanged(wxCommandEvent& event);
    void onFileButtonPressed(wxCommandEvent& event);
    void onRenderSeparationChanged(wxCommandEvent& event);
    void onRenderButtonPressed(wxCommandEvent& event);
    void onOkButtonPressed(wxCommandEvent& event);
    void onCancelButtonPressed(wxCommandEvent& event);
    void onApplyButtonPressed(wxCommandEvent& event);
    void onSetDefaultButtonPressed(wxCommandEvent& event);

    //////////////////////////////////////////////////////////////////////////
    //  model::render::ICodecParameterChangeListener interface
    //////////////////////////////////////////////////////////////////////////

    void onParameterChange() override;

    //////////////////////////////////////////////////////////////////////////
    // TESTS
    //////////////////////////////////////////////////////////////////////////

    wxButton* getFileButton() const;
    EnumSelector<int>* getVideoCodecButton() const;
    EnumSelector<int>* getAudioCodecButton() const;
    wxCheckBox* getRenderSeparationCheckBox() const;
    wxButton* getRenderButton() const;
    wxButton* getOkButton() const;
    wxButton* getCancelButton() const;
    wxButton* getApplyButton() const;
    wxWindow* getAudioParam(model::render::AudioCodecParameterType id) const;
    wxWindow* getVideoParam(model::render::VideoCodecParameterType id) const;

private:

//////////////////////////////////////////////////////////////////////////
// MEMBERS
//////////////////////////////////////////////////////////////////////////

    model::SequencePtr mSequence;
    model::render::RenderPtr mOriginal;
    model::render::RenderPtr mNew;

    wxTextCtrl* mFile;
    wxButton* mFileButton;
    EnumSelector<int>* mVideoCodec;
    EnumSelector<int>* mAudioCodec;
    wxScrolledWindow* mVideoParameters;
    wxScrolledWindow* mAudioParameters;

    wxCheckBox* mRenderSeparation;

    wxButton* mRenderButton;
    wxButton* mOkButton;
    wxButton* mCancelButton;
    wxButton* mApplyButton;
    wxButton* mSetDefaultButton;

    bool mRendering;

    pts mLength;

    std::map<model::render::AudioCodecParameterType, wxWindow*> mAudioParameterWidgets;
    std::map<model::render::VideoCodecParameterType, wxWindow*> mVideoParameterWidgets;

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    void updateAudioCodec();
    void updateVideoCodec();
    void changeAudioCodecInfo(model::render::AudioCodecPtr oldAudioCodec, model::render::AudioCodecPtr newAudioCodec);
    void changeVideoCodecInfo(model::render::VideoCodecPtr oldVideoCodec, model::render::VideoCodecPtr newVideoCodec);
    void enableRenderButton();
    void enableSetDefaultButton();
    bool check();
    void applyNewRender();
};

} // namespace

#endif