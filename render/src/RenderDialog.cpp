#include "RenderDialog.h"

#include <wx/artprov.h>
#include <wx/collpane.h>
#include <wx/filedlg.h>
#include <wx/listbook.h>
#include <wx/msgdlg.h>
#include <wx/panel.h>
#include <wx/propgrid/propgrid.h>
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/function.hpp>
#include <boost/make_shared.hpp>
#include "AudioCodec.h"
#include "AudioCodecs.h"
#include "CodecParameter.h"
#include "Config.h"
#include "Dialog.h"
#include "OutputFormat.h"
#include "OutputFormats.h"
#include "Render.h"
#include "Sequence.h"
#include "UtilFifo.h"
#include "UtilLog.h"
#include "video.xpm" // todo rename
#include "VideoCodec.h"
#include "VideoCodecs.h"
#include "Window.h"
#include "Worker.h"

namespace gui {

static RenderDialog* sCurrent = 0;

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

RenderDialog::RenderDialog(model::SequencePtr sequence)
    :   wxDialog(&Window::get(),wxID_ANY,_("Render sequence"),wxDefaultPosition,wxSize(600,600),wxDEFAULT_DIALOG_STYLE,wxDialogNameStr )
    ,   mSequence(sequence)
    ,   mBook(new wxListbook(this,wxID_ANY,wxDefaultPosition,wxSize(600,600),wxLB_LEFT))
    ,   mRendering(false)
{
    VAR_DEBUG(this);
    sCurrent = this;

    getRender()->Bind(model::render::EVENT_RENDER_PROGRESS, &RenderDialog::onRenderProgress, this);
    getRender()->Bind(model::render::EVENT_RENDER_ACTIVE, &RenderDialog::onRenderActive, this);

    {
        wxPanel* outputbox = new wxPanel(mBook);
        outputbox->SetSizer(new wxFlexGridSizer(3,10,10));

        mFile = new wxTextCtrl(outputbox,wxID_ANY,wxEmptyString,wxDefaultPosition,wxDefaultSize,wxTE_READONLY);
        mFile->Bind(wxEVT_COMMAND_TEXT_UPDATED, &RenderDialog::onFileNameEntered, this);

        mFileButton = new wxButton(outputbox,wxID_ANY,_("Select"));
        mFileButton->Bind(wxEVT_COMMAND_BUTTON_CLICKED, & RenderDialog::onFileButtonPressed, this);

        mVideoCodec = new EnumSelector<int>(outputbox, model::render::VideoCodecs::mapToName, CODEC_ID_NONE);
        mVideoCodec->Bind(wxEVT_COMMAND_CHOICE_SELECTED, &RenderDialog::onVideoCodecChanged, this);

        mAudioCodec = new EnumSelector<int>(outputbox, model::render::AudioCodecs::mapToName, CODEC_ID_NONE);
        mAudioCodec->Bind(wxEVT_COMMAND_CHOICE_SELECTED, &RenderDialog::onAudioCodecChanged, this);

        mRenderButton = new wxButton(outputbox,wxID_ANY,_("Render"));
        mRenderButton->Disable();
        mRenderButton->Bind(wxEVT_COMMAND_BUTTON_CLICKED, & RenderDialog::onRenderButtonPressed, this);

        mLength = mSequence->getLength();

        mProgress = new wxGauge(outputbox,wxID_ANY,static_cast<int>(mLength)); // todo make pts an int

        mProgressText = new wxStaticText(outputbox,wxID_ANY,"");

        outputbox->GetSizer()->Add(new wxStaticText(outputbox,wxID_ANY,_("Output file")));
        outputbox->GetSizer()->Add(mFile,wxSizerFlags().Proportion(1).Expand());
        outputbox->GetSizer()->Add(mFileButton,wxSizerFlags().Proportion(0));

        outputbox->GetSizer()->Add(new wxStaticText(outputbox,wxID_ANY,_("Video codec")));
        outputbox->GetSizer()->Add(mVideoCodec,wxSizerFlags().Proportion(1).Expand());
        outputbox->GetSizer()->AddStretchSpacer();

        outputbox->GetSizer()->Add(new wxStaticText(outputbox,wxID_ANY,_("Audio codec")));
        outputbox->GetSizer()->Add(mAudioCodec,wxSizerFlags().Proportion(1).Expand());
        outputbox->GetSizer()->AddStretchSpacer();

        outputbox->GetSizer()->AddStretchSpacer();
        outputbox->GetSizer()->Add(mRenderButton,wxSizerFlags().Proportion(0));
        outputbox->GetSizer()->AddStretchSpacer();

        outputbox->GetSizer()->AddStretchSpacer();
        outputbox->GetSizer()->Add(mProgress,wxSizerFlags().Proportion(0));
        outputbox->GetSizer()->AddStretchSpacer();

        outputbox->GetSizer()->AddStretchSpacer();
        outputbox->GetSizer()->Add(mProgressText,wxSizerFlags().Proportion(0));
        outputbox->GetSizer()->AddStretchSpacer();

        mBook->AddPage(outputbox,_("Output"));

        // todo add audio codec selector to audio tab, and add video codec selector to render tab (allows to overrule the defaults?)
        // todo file selector uses extension from given format...
    }
    {
        mAudio = new wxPanel(mBook);
        mAudio->SetSizer(new wxBoxSizer(wxVERTICAL));
        mAudio->GetSizer()->Add(new wxButton(mAudio,wxID_ANY,_("AUDIO BUTTON")));
        mBook->AddPage(mAudio,_("Audio"));
    }
    {
        mVideo = new wxPanel(mBook);
        mVideo->SetSizer(new wxBoxSizer(wxVERTICAL));
        mVideo->GetSizer()->Add(new wxButton(mVideo,wxID_ANY,_("safdsf")));
        mBook->AddPage(mVideo,_("Video"));
    }

    const wxSize imageSize(32, 32);
    wxBitmap bmpVideo(video_xpm);
    wxImageList* m_imageList = new wxImageList(imageSize.GetWidth(), imageSize.GetHeight());
    m_imageList->Add(bmpVideo);
    m_imageList->Add(wxArtProvider::GetIcon(wxART_INFORMATION, wxART_OTHER, imageSize));
    m_imageList->Add(wxArtProvider::GetIcon(wxART_QUESTION, wxART_OTHER, imageSize));
    m_imageList->Add(wxArtProvider::GetIcon(wxART_WARNING, wxART_OTHER, imageSize));
    m_imageList->Add(wxArtProvider::GetIcon(wxART_ERROR, wxART_OTHER, imageSize));
    mBook->AssignImageList(m_imageList);
    mBook->SetPageImage(0,0);
    mBook->SetPageImage(1,1);
    mBook->SetPageImage(2,2);

    enableCodecInfo();
    updateOnFileSelection();
    enableRenderButton();

    //Fit();
}

RenderDialog::~RenderDialog()
{
    VAR_DEBUG(this);
    sCurrent = 0;
    mFileButton->Unbind(wxEVT_COMMAND_BUTTON_CLICKED, & RenderDialog::onFileButtonPressed, this);
    mRenderButton->Unbind(wxEVT_COMMAND_BUTTON_CLICKED, & RenderDialog::onRenderButtonPressed, this);
    getRender()->Unbind(model::render::EVENT_RENDER_PROGRESS, &RenderDialog::onRenderProgress, this);
    getRender()->Unbind(model::render::EVENT_RENDER_ACTIVE, &RenderDialog::onRenderActive, this);
    mFile->Unbind(wxEVT_COMMAND_TEXT_UPDATED, &RenderDialog::onFileNameEntered, this);
    mVideoCodec->Unbind(wxEVT_COMMAND_CHOICE_SELECTED, &RenderDialog::onVideoCodecChanged, this);
    mAudioCodec->Unbind(wxEVT_COMMAND_CHOICE_SELECTED, &RenderDialog::onAudioCodecChanged, this);
}

// static
RenderDialog& RenderDialog::get()
{
    ASSERT(sCurrent);
    return *sCurrent;
}

//////////////////////////////////////////////////////////////////////////
// EVENTS
//////////////////////////////////////////////////////////////////////////

void RenderDialog::onRenderProgress(model::render::EventRenderProgress& event) // todo make this getValue a pts not an int
{
    int progress = event.getValue();
    mProgress->SetValue(event.getValue());

    wxString s; s << "Generating frame " << progress << " out of " << mLength;
    mProgressText->SetLabelText(s);

    event.Skip();
}

void RenderDialog::onRenderActive(model::render::EventRenderActive& event)
{
    mRendering = event.getValue();
    mProgress->SetValue(0);
    enableRenderButton();
    event.Skip();
}

void RenderDialog::onVideoCodecChanged(wxCommandEvent& event)
{
    updateOnCodecChange();
    event.Skip();
}

void RenderDialog::onAudioCodecChanged(wxCommandEvent& event)
{
    updateOnCodecChange();
    event.Skip();
}

void RenderDialog::onFileButtonPressed(wxCommandEvent& event)
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

    wxString defaultpath = getRender()->getFileName().IsOk() ? getRender()->getFileName().GetPath() : wxEmptyString;
    wxString defaultfile = getRender()->getFileName().IsOk() ? getRender()->getFileName().GetFullName() : wxEmptyString;
    wxString defaultextension = getRender()->getFileName().IsOk() ? getRender()->getFileName().GetExt() : Config::ReadString(Config::sPathDefaultExtension);
    wxString selected = gui::Dialog::get().getSaveFile(_("Select output file"),filetypes,defaultpath,defaultfile,defaultextension);

    if (!selected.IsEmpty())
    {
        getRender()->setFileName(wxFileName(selected));
    }
    updateOnFileSelection();
    event.Skip();
}

void RenderDialog::onRenderButtonPressed(wxCommandEvent& event)
{
    if (checkFileName(getRender()->getFileName()))
    {
        mRendering = true;
        enableRenderButton();
        WorkPtr work = boost::make_shared<Work>(boost::bind(&model::render::Render::generate,getRender(),mSequence),_("Render sequence"));
        gui::Worker::get().schedule(work);
    }
    else
    {
        wxMessageBox(_("Select output file first."), _("No file selected"), wxOK | wxCENTRE, this);
    }
    event.Skip();
}

void RenderDialog::onFileNameEntered(wxCommandEvent& event)
{
    event.Skip();
}

//////////////////////////////////////////////////////////////////////////
// TESTS
//////////////////////////////////////////////////////////////////////////

wxButton* RenderDialog::getFileButton() const
{
    return mFileButton;
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void RenderDialog::updateOnFileSelection()
{
    model::render::OutputFormatPtr format;
    wxFileName filename = getRender()->getFileName();
    if (filename.IsOk() && filename.HasExt())
    {
        format =  model::render::OutputFormats::getByExtension(filename.GetExt());
    }

    if (format)
    {
        wxConfigBase::Get()->Write(Config::sPathDefaultExtension, filename.GetExt());
        wxConfigBase::Get()->Flush();
        getRender()->setFileName(filename);
        mFile->ChangeValue(getRender()->getFileName().GetFullPath()); // ChangeValue() does not trigger event for text ctrl
        mVideoCodec->select(format->getDefaultVideoCodec());
        mAudioCodec->select(format->getDefaultAudioCodec());
        mVideoCodec->Enable();
        mAudioCodec->Enable();
    }
    else
    {
        getRender()->setFileName(wxFileName());
        mFile->ChangeValue(""); // ChangeValue() does not trigger event for text ctrl
        mVideoCodec->select(CODEC_ID_NONE);
        mAudioCodec->select(CODEC_ID_NONE);
        mVideoCodec->Disable();
        mAudioCodec->Disable();
    }
    updateOnCodecChange();
    enableRenderButton();
}

void RenderDialog::updateOnCodecChange()
{
    getRender()->setVideoCodec(model::render::VideoCodecs::find(static_cast<CodecID>(mVideoCodec->getValue())));
    getRender()->setAudioCodec(model::render::AudioCodecs::find(static_cast<CodecID>(mAudioCodec->getValue())));
    enableCodecInfo();
}

void RenderDialog::enableCodecInfo()
{
    mAudio->DestroyChildren();
    mVideo->DestroyChildren();
    if (getRender()->getAudioCodec())
    {
        mAudio->SetSizer(new wxFlexGridSizer(2));
        BOOST_FOREACH( model::render::ICodecParameterPtr parameter, getRender()->getAudioCodec()->getParameters() )
        {
            mAudio->GetSizer()->Add(new wxStaticText(mAudio,wxID_ANY,parameter->getName()),wxSizerFlags().Proportion(1));
            mAudio->GetSizer()->Add(parameter->makeWidget(mAudio));
        }
        mAudio->Fit();
        mAudio->Enable();
    }
    else
    {
        mAudio->Disable();
    }
    if (getRender()->getVideoCodec())
    {
        mVideo->SetSizer(new wxFlexGridSizer(2));
        BOOST_FOREACH( model::render::ICodecParameterPtr parameter, getRender()->getVideoCodec()->getParameters() )
        {
            mVideo->GetSizer()->Add(new wxStaticText(mVideo,wxID_ANY,parameter->getName()),wxSizerFlags().Proportion(1));
            mVideo->GetSizer()->Add(parameter->makeWidget(mVideo));
        }
        mVideo->Fit();
        mVideo->Enable();
    }
    else
    {
        mVideo->Disable();
    }
}

void RenderDialog::enableRenderButton()
{
    if (getRender()->getFileName().IsOk() && !mRendering)
    {
        mRenderButton->Enable();
    }
    else
    {
        mRenderButton->Disable();
    }
}

bool RenderDialog::checkFileName(wxFileName filename) const
{
    if (!filename.IsOk()) { return false; }
    if (filename.IsDir()) { return false; }
    if (filename.FileExists() && !filename.IsFileWritable()) { return false; }
    return true;
}

model::render::RenderPtr RenderDialog::getRender()
{
    return mSequence->getRender();
}

} //namespace