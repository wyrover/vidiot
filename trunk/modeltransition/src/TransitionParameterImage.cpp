// Copyright 2015-2016 Eric Raijmakers.
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

#include "TransitionParameterImage.h"

#include "Dialog.h"
#include "UtilException.h"
#include "UtilPath.h"

namespace model {

wxString TransitionParameterImage::sParameterImageFilename{ "imagefilename" };

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

TransitionParameterImage::TransitionParameterImage()
    : TransitionParameter()
{
    VAR_DEBUG(*this);
}

TransitionParameterImage::TransitionParameterImage(bool requiresMaskOrAlpha, bool requiresAlpha, const wxString& defaultPath)
    : TransitionParameter()
    , mValue{ }
    , mMaskOrAlpha{ requiresMaskOrAlpha }
    , mAlpha{ requiresAlpha }
    , mDefaultPath{ defaultPath }
{
    VAR_DEBUG(*this);
    ASSERT_IMPLIES(mAlpha, !mMaskOrAlpha);
    ASSERT_IMPLIES(mMaskOrAlpha, !mAlpha);
}

TransitionParameterImage::TransitionParameterImage(const TransitionParameterImage& other)
    : TransitionParameter(other)
    , mValue{ other.mValue }
    , mMaskOrAlpha{ other.mMaskOrAlpha }
    , mAlpha{ other.mAlpha }
    , mDefaultPath{ other.mDefaultPath }
{
    VAR_DEBUG(*this);
}

TransitionParameterImage* TransitionParameterImage::clone() const
{
    return new TransitionParameterImage(static_cast<const TransitionParameterImage&>(*this));
}

TransitionParameterImage::~TransitionParameterImage()
{
    VAR_DEBUG(this);
}

//////////////////////////////////////////////////////////////////////////
// TRANSITIONPARAMETER
//////////////////////////////////////////////////////////////////////////

void TransitionParameterImage::copyValue(TransitionParameterPtr other)
{
    boost::shared_ptr<TransitionParameterImage> typed{ boost::dynamic_pointer_cast<TransitionParameterImage>(other) };
    if (typed)
    {
        setValue(typed->getValue());
    }
}

wxWindow* TransitionParameterImage::makeWidget(wxWindow *parent)    // todo rename to TransitionParameterFilename
{
    ASSERT_ZERO(mPanel);
    ASSERT_ZERO(mFile);
    ASSERT_ZERO(mFileButton);

    mPanel = new wxPanel(parent);
    mPanel->SetSizer(new wxBoxSizer(wxHORIZONTAL));
    mFile = new wxTextCtrl(mPanel, wxID_ANY, mValue.GetFullPath(), wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
    mFile->SetValue(mValue.GetFullPath());
    mFileButton = new wxButton(mPanel, wxID_ANY, _("Select"));
    mFileButton->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &TransitionParameterImage::onFileButtonPressed, this);
    mPanel->GetSizer()->Add(mFile, wxSizerFlags(1).Expand());
    mPanel->GetSizer()->Add(mFileButton, wxSizerFlags(0));

    return mPanel;
}
void TransitionParameterImage::destroyWidget()
{
    ASSERT_NONZERO(mPanel);
    ASSERT_NONZERO(mFile);
    ASSERT_NONZERO(mFileButton);

    mFileButton->Unbind(wxEVT_COMMAND_BUTTON_CLICKED, &TransitionParameterImage::onFileButtonPressed, this);

    mPanel->Destroy();
    mPanel = nullptr;
    mFile = nullptr;
    mFileButton = nullptr;
}
//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

wxFileName TransitionParameterImage::getValue() const 
{ 
    return mValue; 
}

void TransitionParameterImage::setValue(wxFileName value)
{
    if (mValue != value)
    {
        VAR_INFO(value);
        mValue = value;
        if (mFile != nullptr)
        {
            mFile->SetValue(mValue.GetFullPath());
        }
        signalUpdate();
    }
}


//////////////////////////////////////////////////////////////////////////
// GUI EVENTS
//////////////////////////////////////////////////////////////////////////

void TransitionParameterImage::onFileButtonPressed(wxCommandEvent& event)
{
    CatchExceptions([this]
    {                                        
        ASSERT_NONZERO(mPanel);
        static wxString sImage{ "*.bmp;*.gif;*.jpg;*.png;*.tga;*.tif;*.tiff" }; // todo reuse with the string in Dialog...
        static wxString filetypes{ _("Images") + " (" + sImage + ")|" + sImage + ";" + sImage.Upper() };
        wxString selected{ gui::Dialog::get().getFile(_("Select image"), mDefaultPath, filetypes, mPanel) };//todo , defaultpath, defaultfile, defaultextension) };
        if (!selected.IsEmpty())
        {
            // todo checking for mask and/or alpha (and showing a popup in case of missing data!)
            wxFileName value{ util::path::toFileName(selected) };
            value.MakeAbsolute();
            setValue(value);
            // todo checking for file!
        }
    });
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, const TransitionParameterImage& obj)
{
    os << obj.mValue;
    return os;
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void TransitionParameterImage::serialize(Archive & ar, const unsigned int version)
{
    try
    {
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(TransitionParameter);
        ar & BOOST_SERIALIZATION_NVP(mValue);
    }
    catch (boost::exception &e)                  { VAR_ERROR(boost::diagnostic_information(e)); throw; }
    catch (std::exception& e)                    { VAR_ERROR(e.what());                         throw; }
    catch (...)                                  { LOG_ERROR;                                   throw; }
}
template void TransitionParameterImage::serialize<boost::archive::xml_oarchive>(boost::archive::xml_oarchive& ar, const unsigned int archiveVersion);
template void TransitionParameterImage::serialize<boost::archive::xml_iarchive>(boost::archive::xml_iarchive& ar, const unsigned int archiveVersion);

} //namespace

BOOST_CLASS_EXPORT_IMPLEMENT(model::TransitionParameterImage)
