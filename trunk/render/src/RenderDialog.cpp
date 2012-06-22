#include "RenderDialog.h"

#include <wx/artprov.h>
#include <wx/propgrid/propgrid.h>
#include <wx/sizer.h>
#include <wx/panel.h>
#include <wx/statbox.h>
#include <wx/listbook.h>
#include <wx/stattext.h>
#include <wx/collpane.h>
#include <boost/foreach.hpp>
#include "Render.h"
#include "UtilLog.h"
#include "Window.h"
#include "CodecParameter.h"
#include "AudioCodec.h"
#include "VideoCodec.h"
#include "OutputFormat.h"
#include "OutputFormats.h"
#include <wx/filedlg.h>
#include "video.xpm" // todo rename

namespace gui {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

RenderDialog::RenderDialog(model::render::RenderPtr render)
    :   wxDialog(&Window::get(),wxID_ANY,_("Render sequence"),wxDefaultPosition,wxSize(600,600),wxDEFAULT_DIALOG_STYLE,wxDialogNameStr )
    ,   mRender(render)
    ,   mFormat(0)
    ,   mBook(new wxListbook(this,wxID_ANY,wxDefaultPosition,wxSize(600,600),wxLB_LEFT))
{
    VAR_DEBUG(this);

    {
        wxPanel* outputbox = new wxPanel(mBook);
        outputbox->SetSizer(new wxFlexGridSizer(3,10,10));

        mFormat = new wxChoice(outputbox,wxID_ANY);
        BOOST_FOREACH( model::render::OutputFormatPtr format, model::render::OutputFormats::getList() )
        {
            mFormat->Append(format->getLongName());
        }
        mFormat->SetStringSelection(mRender->getOutputFormat()->getLongName());
        mFormat->Bind(wxEVT_COMMAND_CHOICE_SELECTED, &RenderDialog::onFormatChoiceChanged, this);

        mFile = new wxTextCtrl(outputbox,wxID_ANY);
        mFileButton = new wxButton(outputbox,wxID_ANY,_("Select"));
        mFileButton->Bind(wxEVT_COMMAND_BUTTON_CLICKED, & RenderDialog::onFileButtonPressed, this);
        mRenderButton = new wxButton(outputbox,wxID_ANY,_("Render"));
        mRenderButton->Bind(wxEVT_COMMAND_BUTTON_CLICKED, & RenderDialog::onRenderButtonPressed, this);
        mProgress = new wxGauge(outputbox,wxID_ANY,10000);

        outputbox->GetSizer()->Add(new wxStaticText(outputbox,wxID_ANY,_("Output type")));
        outputbox->GetSizer()->Add(mFormat,wxSizerFlags().Proportion(0));
        outputbox->GetSizer()->AddStretchSpacer();

        outputbox->GetSizer()->Add(new wxStaticText(outputbox,wxID_ANY,_("Output file")));
        outputbox->GetSizer()->Add(mFile,wxSizerFlags().Proportion(1).Expand());
        outputbox->GetSizer()->Add(mFileButton,wxSizerFlags().Proportion(0));

        outputbox->GetSizer()->AddStretchSpacer();
        outputbox->GetSizer()->Add(mRenderButton,wxSizerFlags().Proportion(0));
        outputbox->GetSizer()->AddStretchSpacer();

        outputbox->GetSizer()->AddStretchSpacer();
        outputbox->GetSizer()->Add(mProgress,wxSizerFlags().Proportion(0));
        outputbox->GetSizer()->AddStretchSpacer();

        mBook->AddPage(outputbox,_("Output"));
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

    //Fit();
}

RenderDialog::~RenderDialog()
{
    VAR_DEBUG(this);
    mFormat->Unbind(wxEVT_COMMAND_CHOICE_SELECTED, &RenderDialog::onFormatChoiceChanged, this);
    mFileButton->Unbind(wxEVT_COMMAND_BUTTON_CLICKED, & RenderDialog::onFileButtonPressed, this);
    mRenderButton->Unbind(wxEVT_COMMAND_BUTTON_CLICKED, & RenderDialog::onRenderButtonPressed, this);
}

//////////////////////////////////////////////////////////////////////////
// EVENTS
//////////////////////////////////////////////////////////////////////////

void RenderDialog::onFormatChoiceChanged(wxCommandEvent& event)
{
    mRender->setOutputFormat(model::render::OutputFormats::getByName(mFormat->GetStringSelection()));
    enableCodecInfo();
    event.Skip();
}

void RenderDialog::onFileButtonPressed(wxCommandEvent& event)
{
    wxString filename = wxFileSelector("Choose a file to open");
    if ( !filename.empty() )
    {
        mFile->SetValue(filename);
        // work with the file
    }
    //else: cancelled by user

    event.Skip();
}

void RenderDialog::onRenderButtonPressed(wxCommandEvent& event)
{
    event.Skip();
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void RenderDialog::enableCodecInfo()
{
    mAudio->DestroyChildren();
    mVideo->DestroyChildren();
    if (mRender->getOutputFormat()->getAudioCodec())
    {
        mAudio->SetSizer(new wxFlexGridSizer(2));
        BOOST_FOREACH( model::render::ICodecParameterPtr parameter, mRender->getOutputFormat()->getAudioCodec()->getParameters() )
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
    if (mRender->getOutputFormat()->getVideoCodec())
    {
        mVideo->SetSizer(new wxFlexGridSizer(2));
        BOOST_FOREACH( model::render::ICodecParameterPtr parameter, mRender->getOutputFormat()->getVideoCodec()->getParameters() )
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

} //namespace