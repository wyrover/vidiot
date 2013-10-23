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

#include "DialogRenderSettings.h"

#include "AudioCodec.h"
#include "AudioCodecs.h"
#include "CodecParameter.h"
#include "Config.h"
#include "Dialog.h"
#include "OutputFormat.h"
#include "OutputFormats.h"
#include "ProjectModification.h"
#include "Properties.h"
#include "Render.h"
#include "Sequence.h"
#include "UtilCloneable.h"
#include "UtilFifo.h"
#include "UtilLog.h"
#include "UtilLogWxwidgets.h"
#include "VideoCodec.h"
#include "VideoCodecs.h"
#include "Window.h"
#include "Worker.h"

namespace gui {

wxString sIncompatibleHeader(_("Incompatible codec and file type"));

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

void addOption(wxWindow* parent, wxSizer* vSizer, wxString name, wxWindow* option)
{
    wxStaticText* wxst = new wxStaticText(parent,wxID_ANY,name,wxDefaultPosition,wxDefaultSize,wxST_ELLIPSIZE_MIDDLE);
    wxst->SetMinSize(wxSize(120,-1));
    wxst->SetSize(wxSize(120,-1));
    wxBoxSizer* hSizer = new wxBoxSizer(wxHORIZONTAL);
    hSizer->Add(wxst,wxSizerFlags(1));
    hSizer->Add(option,wxSizerFlags(2).Right());
    vSizer->Add(hSizer,wxSizerFlags().Expand());
};

DialogRenderSettings::DialogRenderSettings(model::SequencePtr sequence)
    :   wxDialog(&Window::get(),wxID_ANY,_("Render sequence"),wxDefaultPosition,wxSize(600,600),wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER,wxDialogNameStr )
    ,   mSequence(sequence)
    ,   mOriginal(make_cloned<model::render::Render>(sequence->getRender()))
    ,   mNew(make_cloned<model::render::Render>(sequence->getRender()))
    ,   mFile(0)
    ,   mFileButton(0)
    ,   mVideoCodec(0)
    ,   mAudioCodec(0)
    ,   mRenderButton(0)
    ,   mOkButton(0)
    ,   mCancelButton(0)
    ,   mApplyButton(0)
    ,   mSetDefaultButton(0)
    ,   mRendering(false)
    ,   mLength(0)
{
    VAR_DEBUG(this);

    mLength = mSequence->getLength();
    SetSizer(new wxBoxSizer(wxVERTICAL));

    wxPanel* outputbox = new wxPanel(this,wxID_ANY,wxDefaultPosition,wxDefaultSize,wxBORDER_RAISED);
    outputbox->SetSizer(new wxBoxSizer(wxVERTICAL));

    //////// FORMAT ////////

    wxStaticBox* formatbox = new wxStaticBox(outputbox,wxID_ANY,_("Format"));
    wxStaticBoxSizer* formatboxsizer = new wxStaticBoxSizer(formatbox, wxVERTICAL);

    wxPanel* fileselect = new wxPanel(formatbox);
    fileselect->SetSizer(new wxBoxSizer(wxHORIZONTAL));
    mFile = new wxTextCtrl(fileselect,wxID_ANY,mNew->getFileName().GetFullPath(),wxDefaultPosition,wxDefaultSize,wxTE_READONLY);
    mFileButton = new wxButton(fileselect,wxID_ANY,_("Select"));
    mFileButton->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &DialogRenderSettings::onFileButtonPressed, this);
    fileselect->GetSizer()->Add(mFile,wxSizerFlags(1).Expand());
    fileselect->GetSizer()->Add(mFileButton,wxSizerFlags(0));

    mVideoCodec = new EnumSelector<int>(formatbox, model::render::VideoCodecs::mapToName, mNew->getOutputFormat()->getVideoCodec()->getId());
    mVideoCodec->Bind(wxEVT_COMMAND_CHOICE_SELECTED, &DialogRenderSettings::onVideoCodecChanged, this);

    mAudioCodec = new EnumSelector<int>(formatbox, model::render::AudioCodecs::mapToName, mNew->getOutputFormat()->getAudioCodec()->getId());
    mAudioCodec->Bind(wxEVT_COMMAND_CHOICE_SELECTED, &DialogRenderSettings::onAudioCodecChanged, this);

    addOption(formatbox,formatboxsizer,_("Output file"), fileselect);
    addOption(formatbox,formatboxsizer,_("Video codec"), mVideoCodec);
    addOption(formatbox,formatboxsizer,_("Audio codec"), mAudioCodec);

    //////// VIDEO ////////

    wxStaticBoxSizer* mVideoBoxSizer = new wxStaticBoxSizer(wxVERTICAL,outputbox,_("Video settings"));
    mVideoParameters = new wxScrolledWindow(mVideoBoxSizer->GetStaticBox());
    mVideoBoxSizer->Add(mVideoParameters,wxSizerFlags(1).Expand());

    //////// AUDIO ////////

    wxStaticBoxSizer* mAudioBoxSizer = new wxStaticBoxSizer(wxVERTICAL,outputbox,_("Audio settings"));
    mAudioParameters = new wxScrolledWindow(mAudioBoxSizer->GetStaticBox());
    mAudioBoxSizer->Add(mAudioParameters,wxSizerFlags(1).Expand());

    //////// OPTIONS ////////

    wxStaticBox* optionsbox = new wxStaticBox(outputbox,wxID_ANY,_("Options"));
    wxStaticBoxSizer* optionsboxsizer = new wxStaticBoxSizer(optionsbox, wxHORIZONTAL);

    mRenderSeparation = new wxCheckBox(optionsbox, wxID_ANY, "");
    mRenderSeparation->SetValue(mNew->getSeparateAtCuts());
    mRenderSeparation->Bind(wxEVT_COMMAND_CHECKBOX_CLICKED, &DialogRenderSettings::onRenderSeparationChanged, this);

    addOption(optionsbox,optionsboxsizer,_("Render separation between cuts"), mRenderSeparation);

    //////// ACTIONS ////////

    wxStaticBox* actionbox = new wxStaticBox(outputbox,wxID_ANY,_("Actions"));
    wxStaticBoxSizer* actionboxsizer = new wxStaticBoxSizer(actionbox, wxHORIZONTAL);

    mSetDefaultButton = new wxButton(actionbox,wxID_ANY,_("Set as default"));
    mSetDefaultButton->Bind(wxEVT_COMMAND_BUTTON_CLICKED, & DialogRenderSettings::onSetDefaultButtonPressed, this);

    actionboxsizer->Add(mSetDefaultButton,wxSizerFlags().Proportion(0));

    //////// BUTTONS ////////

    wxPanel* buttons = new wxPanel(this);
    buttons->SetSizer(new wxBoxSizer(wxHORIZONTAL));
    mRenderButton = new wxButton(buttons,wxID_ANY,_("OK && Render now"));
    mOkButton = new wxButton(buttons,wxID_ANY,_("OK"));
    mCancelButton = new wxButton(buttons,wxID_CANCEL,_("Cancel")); // Using wxID_CANCEL ensures that pressing ESC also closes the dialog
    mApplyButton = new wxButton(buttons,wxID_ANY,_("Apply"));

    mRenderButton->Bind(wxEVT_COMMAND_BUTTON_CLICKED, & DialogRenderSettings::onRenderButtonPressed, this);
    mOkButton->Bind(wxEVT_COMMAND_BUTTON_CLICKED, & DialogRenderSettings::onOkButtonPressed, this);
    mCancelButton->Bind(wxEVT_COMMAND_BUTTON_CLICKED, & DialogRenderSettings::onCancelButtonPressed, this);
    mApplyButton->Bind(wxEVT_COMMAND_BUTTON_CLICKED, & DialogRenderSettings::onApplyButtonPressed, this);

    buttons->GetSizer()->Add(mRenderButton);
    buttons->GetSizer()->Add(mOkButton);
    buttons->GetSizer()->Add(mCancelButton);
    buttons->GetSizer()->Add(mApplyButton);

    ////////  ////////

    outputbox->GetSizer()->Add(formatboxsizer,wxSizerFlags(0).Expand());
    outputbox->GetSizer()->Add(mVideoBoxSizer,wxSizerFlags(1).Expand());
    outputbox->GetSizer()->Add(mAudioBoxSizer,wxSizerFlags(1).Expand());
    outputbox->GetSizer()->Add(optionsboxsizer,wxSizerFlags(0).Expand());
    outputbox->GetSizer()->Add(actionboxsizer,wxSizerFlags(0));

    GetSizer()->Add(outputbox,wxSizerFlags(1).Top().Right().Expand().Border());
    GetSizer()->Add(buttons,wxSizerFlags(0).Bottom().Right().Border());

    changeAudioCodecInfo(model::render::AudioCodecPtr(), mNew->getOutputFormat()->getAudioCodec());
    changeVideoCodecInfo(model::render::VideoCodecPtr(), mNew->getOutputFormat()->getVideoCodec());

    enableSetDefaultButton();
    enableRenderButton();
}

DialogRenderSettings::~DialogRenderSettings()
{
    VAR_DEBUG(this);
    mRenderSeparation->Unbind(wxEVT_COMMAND_CHECKBOX_CLICKED, &DialogRenderSettings::onRenderSeparationChanged, this);
    mFileButton->Unbind(wxEVT_COMMAND_BUTTON_CLICKED, & DialogRenderSettings::onFileButtonPressed, this);
    mRenderButton->Unbind(wxEVT_COMMAND_BUTTON_CLICKED, & DialogRenderSettings::onRenderButtonPressed, this);
    mOkButton->Unbind(wxEVT_COMMAND_BUTTON_CLICKED, & DialogRenderSettings::onOkButtonPressed, this);
    mCancelButton->Unbind(wxEVT_COMMAND_BUTTON_CLICKED, & DialogRenderSettings::onCancelButtonPressed, this);
    mApplyButton->Unbind(wxEVT_COMMAND_BUTTON_CLICKED, & DialogRenderSettings::onApplyButtonPressed, this);
    mSetDefaultButton->Unbind(wxEVT_COMMAND_BUTTON_CLICKED, & DialogRenderSettings::onSetDefaultButtonPressed, this);
    mVideoCodec->Unbind(wxEVT_COMMAND_CHOICE_SELECTED, &DialogRenderSettings::onVideoCodecChanged, this);
    mAudioCodec->Unbind(wxEVT_COMMAND_CHOICE_SELECTED, &DialogRenderSettings::onAudioCodecChanged, this);
}

//////////////////////////////////////////////////////////////////////////
// EVENTS
//////////////////////////////////////////////////////////////////////////

void DialogRenderSettings::onVideoCodecChanged(wxCommandEvent& event)
{
    updateVideoCodec();
    event.Skip();
}

void DialogRenderSettings::onAudioCodecChanged(wxCommandEvent& event)
{
    updateAudioCodec();
    event.Skip();
}

void DialogRenderSettings::onFileButtonPressed(wxCommandEvent& event)
{
    wxString filetypes;
    for ( model::render::OutputFormatPtr format : model::render::OutputFormats::getList() )
    {
        ASSERT(!format->getExtensions().empty());
        wxString extensionlist;
        for ( wxString extension : format->getExtensions() )
        {
            if (!extensionlist.IsEmpty()) { extensionlist << ';'; } // Add ; after each extension except the first
            extensionlist << "*." << extension;
        }
        if (!filetypes.IsEmpty()) { filetypes << "|"; } // Add | after each file type except the first
        filetypes << format->getLongName() << " (" << extensionlist << ")|" << extensionlist;
    }

    wxString defaultpath = mNew->getFileName().IsOk() ? mNew->getFileName().GetPath() : wxEmptyString;
    wxString defaultfile = mNew->getFileName().IsOk() ? mNew->getFileName().GetFullName() : wxEmptyString;
    wxString defaultextension = mNew->getFileName().IsOk() ? mNew->getFileName().GetExt() : Config::ReadString(Config::sPathDefaultExtension);
    wxString selected = gui::Dialog::get().getSaveFile(_("Select output file"),filetypes,defaultpath,defaultfile,defaultextension);

    if (!selected.IsEmpty())
    {
        wxFileName oldName = mNew->getFileName();
        wxFileName newName = wxFileName(selected);

        if (newName.IsOk() && newName.HasExt())
        {
            mNew->setFileName(newName);
            if (!newName.GetExt().IsSameAs(oldName.GetExt()))
            {
                // Set default codecs for the chosen file format
                model::render::OutputFormatPtr format;
                format =  model::render::OutputFormats::getByExtension(newName.GetExt());
                ASSERT(format)(newName);
                Config::WriteString(Config::sPathDefaultExtension, newName.GetExt());
                mFile->ChangeValue(newName.GetFullPath()); // ChangeValue() does not trigger event for text ctrl
                mVideoCodec->select(format->getDefaultVideoCodec());
                mAudioCodec->select(format->getDefaultAudioCodec());
                updateVideoCodec();
                updateAudioCodec();
            }

            mFile->ChangeValue(selected);
        }
    }
    enableRenderButton();
    event.Skip();
}

void DialogRenderSettings::onRenderSeparationChanged(wxCommandEvent& event)
{
    mNew->setSeparateAtCuts(mRenderSeparation->GetValue());
    event.Skip();
}

void DialogRenderSettings::onRenderButtonPressed(wxCommandEvent& event)
{
    if (check())
    {
        applyNewRender();
        mRendering = true;
        model::render::Render::schedule(mSequence);
        Close();
    }
    event.Skip();
}

void DialogRenderSettings::onOkButtonPressed(wxCommandEvent& event)
{
    if (check())
    {
        applyNewRender();
        Close();
    }
    event.Skip();
}

void DialogRenderSettings::onCancelButtonPressed(wxCommandEvent& event)
{
    Close();
    event.Skip();
}

void DialogRenderSettings::onApplyButtonPressed(wxCommandEvent& event)
{
    if (check())
    {
        applyNewRender();
    }
    event.Skip();
}

void DialogRenderSettings::onSetDefaultButtonPressed(wxCommandEvent& event)
{
    model::Properties::get().setDefaultRender(mNew);
    enableSetDefaultButton();
    event.Skip();
}

//////////////////////////////////////////////////////////////////////////
//  model::render::ICodecParameterChangeListener interface
//////////////////////////////////////////////////////////////////////////

void DialogRenderSettings::onParameterChange()
{
    enableSetDefaultButton();
}

//////////////////////////////////////////////////////////////////////////
// TESTS
//////////////////////////////////////////////////////////////////////////

wxButton* DialogRenderSettings::getFileButton() const
{
    ASSERT(mFileButton);
    return mFileButton;
}

EnumSelector<int>* DialogRenderSettings::getVideoCodecButton() const
{
    return mVideoCodec;
}

wxCheckBox* DialogRenderSettings::getRenderSeparationCheckBox() const
{
    return mRenderSeparation;
}

wxButton* DialogRenderSettings::getRenderButton() const
{
    return mRenderButton;
}

wxButton* DialogRenderSettings::getOkButton() const
{
    return mOkButton;
}

wxButton* DialogRenderSettings::getCancelButton() const
{
    return mCancelButton;
}

wxButton* DialogRenderSettings::getApplyButton() const
{
    return mApplyButton;
}

wxWindow* DialogRenderSettings::getAudioParam(int index) const
{
    return mAudioParameterWidgets[index];
}

wxWindow* DialogRenderSettings::getVideoParam(int index) const
{
    return mVideoParameterWidgets[index];
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void DialogRenderSettings::updateAudioCodec()
{
    model::render::AudioCodecPtr old = mNew->getOutputFormat()->getAudioCodec();
    CodecID newCodecID = static_cast<CodecID>(mAudioCodec->getValue());
    bool useNewCodecID = true;
    if (mNew->getOutputFormat()->checkCodec(newCodecID) == 0)
    {
        gui::Dialog::get().getConfirmation(sIncompatibleHeader,_("This audio codec can not be stored in the given file type"));
        useNewCodecID = false;
    }
    else if (mNew->getOutputFormat()->checkCodec(newCodecID) < 0)
    {
        int result = gui::Dialog::get().getConfirmation(sIncompatibleHeader,_("This audio codec may cause problems with the given file type (cannot determine if this will work properly)."), wxOK | wxCANCEL);
        useNewCodecID = (result == wxOK);
    }
    if (useNewCodecID)
    {
        mNew->getOutputFormat()->setAudioCodec(model::render::AudioCodecs::find(static_cast<CodecID>(mAudioCodec->getValue())));
        changeAudioCodecInfo(old, mNew->getOutputFormat()->getAudioCodec());
        enableSetDefaultButton();
    }
    else
    {
        mAudioCodec->select(old->getId());
    }
}

void DialogRenderSettings::updateVideoCodec()
{
    model::render::VideoCodecPtr old = mNew->getOutputFormat()->getVideoCodec();
    CodecID newCodecID = static_cast<CodecID>(mVideoCodec->getValue());
    bool useNewCodecID = true;
    if (mNew->getOutputFormat()->checkCodec(newCodecID) == 0)
    {
        gui::Dialog::get().getConfirmation(sIncompatibleHeader,_("This video codec can not be stored in the given file type"));
        useNewCodecID = false;
    }
    else if (mNew->getOutputFormat()->checkCodec(newCodecID) < 0)
    {
        int result = gui::Dialog::get().getConfirmation(sIncompatibleHeader,_("This video codec may cause problems with the given file type (cannot determine if this will work properly)."), wxOK | wxCANCEL);
        useNewCodecID = (result == wxOK);
    }
    if (useNewCodecID)
    {
        mNew->getOutputFormat()->setVideoCodec(model::render::VideoCodecs::find(static_cast<CodecID>(mVideoCodec->getValue())));
        changeVideoCodecInfo(old, mNew->getOutputFormat()->getVideoCodec());
        enableSetDefaultButton();
    }
    else
    {
        mVideoCodec->select(old->getId());
    }
}

void DialogRenderSettings::changeAudioCodecInfo(model::render::AudioCodecPtr oldAudioCodec, model::render::AudioCodecPtr newAudioCodec)
{
    mAudioParameters->Disable();
    if (oldAudioCodec)
    {
        mVideoParameterWidgets.clear();
        for ( model::render::ICodecParameterPtr parameter : oldAudioCodec->getParameters() )
        {
            parameter->destroyWidget();
        }
         mAudioParameters->DestroyChildren();
    }
    if (newAudioCodec)
    {
        wxSizer* vSizer = new wxBoxSizer(wxVERTICAL);
        for ( model::render::ICodecParameterPtr parameter : mNew->getOutputFormat()->getAudioCodec()->getParameters() )
        {
            wxWindow* window = parameter->makeWidget(mAudioParameters,this);
            addOption(mAudioParameters,vSizer,parameter->getName(),window);
            mAudioParameterWidgets.push_back(window);
        }
        mAudioParameters->SetSizer(vSizer);
        mAudioParameters->Layout();
        mAudioParameters->Enable();
    }
}

void DialogRenderSettings::changeVideoCodecInfo(model::render::VideoCodecPtr oldVideoCodec, model::render::VideoCodecPtr newVideoCodec)
{
    mVideoParameters->Disable();
    if (oldVideoCodec)
    {
        mVideoParameterWidgets.clear();
        for ( model::render::ICodecParameterPtr parameter : oldVideoCodec->getParameters() )
        {
            parameter->destroyWidget();
        }
        mVideoParameters->DestroyChildren();
    }
    if (newVideoCodec)
    {
        wxSizer* vSizer = new wxBoxSizer(wxVERTICAL);
        for ( model::render::ICodecParameterPtr parameter : mNew->getOutputFormat()->getVideoCodec()->getParameters() )
        {
            wxWindow* window = parameter->makeWidget(mVideoParameters,this);
            addOption(mVideoParameters,vSizer,parameter->getName(),window);
            mVideoParameterWidgets.push_back(window);
        }
        mVideoParameters->SetSizer(vSizer);
        mVideoParameters->Layout();
        mVideoParameters->Enable();
    }
}

void DialogRenderSettings::enableRenderButton()
{
    if (mNew->getFileName().IsOk() && !mRendering)
    {
        mRenderButton->Enable();
    }
    else
    {
        mRenderButton->Disable();
    }
}

void DialogRenderSettings::enableSetDefaultButton()
{
    mSetDefaultButton->Enable();
    if (*(model::Properties::get().getDefaultRender()->withFileNameRemoved()) == *(mNew->withFileNameRemoved()))
    {
        mSetDefaultButton->Disable();
    }
}

bool DialogRenderSettings::check()
{
    if (!mNew->checkFileName())
    {
        gui::Dialog::get().getConfirmation(_("Select output file first."), _("No file selected"));
        return false;
    }
    if (!mNew->getOutputFormat()->storeVideo() && !mNew->getOutputFormat()->storeAudio())
    {
        gui::Dialog::get().getConfirmation(_("Nothing to render"),_("Select at least a video or audio codec."));
        return false;
    }
    return true;
}

void DialogRenderSettings::applyNewRender()
{
    ASSERT(check());
    mSequence->setRender(make_cloned<model::render::Render>(mNew));
    mOriginal = make_cloned<model::render::Render>(mNew);
    model::ProjectModification::trigger();
    ASSERT_EQUALS(*mOriginal,*mNew);
}

} //namespace