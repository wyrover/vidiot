#include "RenderSettingsDialog.h"

#include "AudioCodec.h"
#include "AudioCodecs.h"
#include "CodecParameter.h"
#include "Config.h"
#include "Dialog.h"
#include "OutputFormat.h"
#include "OutputFormats.h"
#include "Project.h"
#include "Properties.h"
#include "Render.h"
#include "Sequence.h"
#include "UtilFifo.h"
#include "UtilLog.h"
#include "UtilLogWxwidgets.h"
#include "VideoCodec.h"
#include "VideoCodecs.h"
#include "Window.h"
#include "Worker.h"

namespace gui {

static RenderSettingsDialog* sCurrent = 0;

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

void addOption(wxWindow* parent, wxSizer* vSizer, wxString name, wxWindow* option)
{
    wxStaticText* wxst = new wxStaticText(parent,wxID_ANY,name);
    wxst->SetMinSize(wxSize(100,-1));
    wxBoxSizer* hSizer = new wxBoxSizer(wxHORIZONTAL);
    hSizer->Add(wxst,wxSizerFlags(1));
    hSizer->Add(option,wxSizerFlags(2));
    vSizer->Add(hSizer,wxSizerFlags().Expand());
};

RenderSettingsDialog::RenderSettingsDialog(model::SequencePtr sequence)
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
    sCurrent = this;

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
    mFileButton->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &RenderSettingsDialog::onFileButtonPressed, this);
    fileselect->GetSizer()->Add(mFile,wxSizerFlags(1).Expand());
    fileselect->GetSizer()->Add(mFileButton,wxSizerFlags(0));

    mVideoCodec = new EnumSelector<int>(formatbox, model::render::VideoCodecs::mapToName, mNew->getVideoCodec()->getId());
    mVideoCodec->Bind(wxEVT_COMMAND_CHOICE_SELECTED, &RenderSettingsDialog::onVideoCodecChanged, this);

    mAudioCodec = new EnumSelector<int>(formatbox, model::render::AudioCodecs::mapToName, mNew->getAudioCodec()->getId());
    mAudioCodec->Bind(wxEVT_COMMAND_CHOICE_SELECTED, &RenderSettingsDialog::onAudioCodecChanged, this);

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

    //////// ACTIONS ////////

    wxStaticBox* actionbox = new wxStaticBox(outputbox,wxID_ANY,_("Actions"));
    wxStaticBoxSizer* actionboxsizer = new wxStaticBoxSizer(actionbox, wxHORIZONTAL);

    mSetDefaultButton = new wxButton(actionbox,wxID_ANY,_("Set as default"));
    mSetDefaultButton->Bind(wxEVT_COMMAND_BUTTON_CLICKED, & RenderSettingsDialog::onSetDefaultButtonPressed, this);

    actionboxsizer->Add(mSetDefaultButton,wxSizerFlags().Proportion(0));

    //////// BUTTONS ////////

    wxPanel* buttons = new wxPanel(this);
    buttons->SetSizer(new wxBoxSizer(wxHORIZONTAL));
    mRenderButton = new wxButton(buttons,wxID_ANY,_("OK && Render now"));
    mOkButton = new wxButton(buttons,wxID_ANY,_("OK"));
    mCancelButton = new wxButton(buttons,wxID_ANY,_("Cancel"));
    mApplyButton = new wxButton(buttons,wxID_ANY,_("Apply"));

    mRenderButton->Bind(wxEVT_COMMAND_BUTTON_CLICKED, & RenderSettingsDialog::onRenderButtonPressed, this);
    mOkButton->Bind(wxEVT_COMMAND_BUTTON_CLICKED, & RenderSettingsDialog::onOkButtonPressed, this);
    mCancelButton->Bind(wxEVT_COMMAND_BUTTON_CLICKED, & RenderSettingsDialog::onCancelButtonPressed, this);
    mApplyButton->Bind(wxEVT_COMMAND_BUTTON_CLICKED, & RenderSettingsDialog::onApplyButtonPressed, this);

    buttons->GetSizer()->Add(mRenderButton);
    buttons->GetSizer()->Add(mOkButton);
    buttons->GetSizer()->Add(mCancelButton);
    buttons->GetSizer()->Add(mApplyButton);

    ////////  ////////

    outputbox->GetSizer()->Add(formatboxsizer,wxSizerFlags(0).Expand());
    outputbox->GetSizer()->Add(mVideoBoxSizer,wxSizerFlags(1).Expand());
    outputbox->GetSizer()->Add(mAudioBoxSizer,wxSizerFlags(1).Expand());
    outputbox->GetSizer()->Add(actionboxsizer,wxSizerFlags(0));

    GetSizer()->Add(outputbox,wxSizerFlags(1).Top().Right().Expand().Border());
    GetSizer()->Add(buttons,wxSizerFlags(0).Bottom().Right().Border());

    changeAudioCodecInfo(model::render::AudioCodecPtr(), mNew->getAudioCodec());
    changeVideoCodecInfo(model::render::VideoCodecPtr(), mNew->getVideoCodec());

    enableSetDefaultButton();
    enableRenderButton();
}

RenderSettingsDialog::~RenderSettingsDialog()
{
    VAR_DEBUG(this);
    if (*mOriginal != *mNew)
    {
        // This causes a 'save' dialog when closing.
        // KP: Change render options, then undo until all items removed. Then close. Save as dialog will not be shown.
        model::Project::get().Modify(true);
    }
    sCurrent = 0;
    mFileButton->Unbind(wxEVT_COMMAND_BUTTON_CLICKED, & RenderSettingsDialog::onFileButtonPressed, this);
    mRenderButton->Unbind(wxEVT_COMMAND_BUTTON_CLICKED, & RenderSettingsDialog::onRenderButtonPressed, this);
    mOkButton->Unbind(wxEVT_COMMAND_BUTTON_CLICKED, & RenderSettingsDialog::onOkButtonPressed, this);
    mCancelButton->Unbind(wxEVT_COMMAND_BUTTON_CLICKED, & RenderSettingsDialog::onCancelButtonPressed, this);
    mApplyButton->Unbind(wxEVT_COMMAND_BUTTON_CLICKED, & RenderSettingsDialog::onApplyButtonPressed, this);
    mSetDefaultButton->Unbind(wxEVT_COMMAND_BUTTON_CLICKED, & RenderSettingsDialog::onSetDefaultButtonPressed, this);
    mVideoCodec->Unbind(wxEVT_COMMAND_CHOICE_SELECTED, &RenderSettingsDialog::onVideoCodecChanged, this);
    mAudioCodec->Unbind(wxEVT_COMMAND_CHOICE_SELECTED, &RenderSettingsDialog::onAudioCodecChanged, this);
}

// static
RenderSettingsDialog& RenderSettingsDialog::get()
{
    ASSERT(sCurrent);
    return *sCurrent;
}

//////////////////////////////////////////////////////////////////////////
// EVENTS
//////////////////////////////////////////////////////////////////////////

void RenderSettingsDialog::onVideoCodecChanged(wxCommandEvent& event)
{
    updateVideoCodec();
    event.Skip();
}

void RenderSettingsDialog::onAudioCodecChanged(wxCommandEvent& event)
{
    updateAudioCodec();
    event.Skip();
}

void RenderSettingsDialog::onFileButtonPressed(wxCommandEvent& event)
{
    wxString filetypes;
    BOOST_FOREACH( model::render::OutputFormatPtr format, model::render::OutputFormats::getList() )
    {
        ASSERT(!format->getExtensions().empty());
        wxString extensionlist;
        BOOST_FOREACH( wxString extension, format->getExtensions() )
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
                wxConfigBase::Get()->Write(Config::sPathDefaultExtension, newName.GetExt());
                wxConfigBase::Get()->Flush();
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

void RenderSettingsDialog::onRenderButtonPressed(wxCommandEvent& event)
{
    if (checkFilename())
    {
        onApplyButtonPressed(event);
        mRendering = true;
        enableRenderButton();
        model::render::Render::schedule(mSequence);
        model::render::Render::schedule(mSequence);
        model::render::Render::schedule(mSequence);
        Close();
    }
    event.Skip();
}

void RenderSettingsDialog::onOkButtonPressed(wxCommandEvent& event)
{
    onApplyButtonPressed(event);
    Close();
    event.Skip();
}

void RenderSettingsDialog::onCancelButtonPressed(wxCommandEvent& event)
{
    Close();
    event.Skip();
}

void RenderSettingsDialog::onApplyButtonPressed(wxCommandEvent& event)
{
    if (checkFilename())
    {
        mSequence->setRender(make_cloned<model::render::Render>(mNew));
        mOriginal = make_cloned<model::render::Render>(mNew);
    }
    event.Skip();
}

void RenderSettingsDialog::onSetDefaultButtonPressed(wxCommandEvent& event)
{
    model::Properties::get()->setDefaultRender(mNew);
    enableSetDefaultButton();
    event.Skip();
}

//////////////////////////////////////////////////////////////////////////
//  model::render::ICodecParameterChangeListener interface
//////////////////////////////////////////////////////////////////////////

void RenderSettingsDialog::onParameterChange()
{
    enableSetDefaultButton();
}

//////////////////////////////////////////////////////////////////////////
// TESTS
//////////////////////////////////////////////////////////////////////////

wxButton* RenderSettingsDialog::getFileButton() const
{
    ASSERT(mFileButton);
    return mFileButton;
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void RenderSettingsDialog::updateAudioCodec()
{
    model::render::AudioCodecPtr old = mNew->getAudioCodec();
    mNew->setAudioCodec(model::render::AudioCodecs::find(static_cast<CodecID>(mAudioCodec->getValue())));
    changeAudioCodecInfo(old, mNew->getAudioCodec());
    enableSetDefaultButton();
}

void RenderSettingsDialog::updateVideoCodec()
{
    model::render::VideoCodecPtr old = mNew->getVideoCodec();
    mNew->setVideoCodec(model::render::VideoCodecs::find(static_cast<CodecID>(mVideoCodec->getValue())));
    changeVideoCodecInfo(old, mNew->getVideoCodec());
    enableSetDefaultButton();
}

void RenderSettingsDialog::changeAudioCodecInfo(model::render::AudioCodecPtr oldAudioCodec, model::render::AudioCodecPtr newAudioCodec)
{
    mAudioParameters->Disable();
    if (oldAudioCodec)
    {
        BOOST_FOREACH( model::render::ICodecParameterPtr parameter, oldAudioCodec->getParameters() )
        {
            parameter->destroyWidget();
        }
         mAudioParameters->DestroyChildren();
    }
    if (newAudioCodec)
    {
        wxSizer* vSizer = new wxBoxSizer(wxVERTICAL);
        BOOST_FOREACH( model::render::ICodecParameterPtr parameter, mNew->getAudioCodec()->getParameters() )
        {
            addOption(mAudioParameters,vSizer,parameter->getName(),parameter->makeWidget(mAudioParameters,this));
        }
        mAudioParameters->SetSizer(vSizer);
        mAudioParameters->Layout();
        mAudioParameters->Enable();

    }
}

void RenderSettingsDialog::changeVideoCodecInfo(model::render::VideoCodecPtr oldVideoCodec, model::render::VideoCodecPtr newVideoCodec)
{
    mVideoParameters->Disable();
    if (oldVideoCodec)
    {
        BOOST_FOREACH( model::render::ICodecParameterPtr parameter, oldVideoCodec->getParameters() )
        {
            parameter->destroyWidget();
        }
        mVideoParameters->DestroyChildren();
    }
    if (newVideoCodec)
    {
        wxSizer* vSizer = new wxBoxSizer(wxVERTICAL);
        BOOST_FOREACH( model::render::ICodecParameterPtr parameter, mNew->getVideoCodec()->getParameters() )
        {
            addOption(mVideoParameters,vSizer,parameter->getName(),parameter->makeWidget(mVideoParameters,this));
        }
        mVideoParameters->SetSizer(vSizer);
        mVideoParameters->Layout();
        mVideoParameters->Enable();
    }
}

void RenderSettingsDialog::enableRenderButton()
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

void RenderSettingsDialog::enableSetDefaultButton()
{
    mSetDefaultButton->Enable();
    if (*(model::Properties::get()->getDefaultRender()->withFileNameRemoved()) == *(mNew->withFileNameRemoved()))
    {
        mSetDefaultButton->Disable();
    }
}

bool RenderSettingsDialog::checkFilename()
{
    if (!mNew->checkFileName())
    {
        wxMessageBox(_("Select output file first."), _("No file selected"), wxOK | wxCENTRE, this);
        return false;
    }
    return true;
}

} //namespace