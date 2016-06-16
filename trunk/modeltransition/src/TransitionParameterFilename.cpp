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

#include "TransitionParameterFilename.h"

#include "Dialog.h"
#include "UtilException.h"
#include "UtilPath.h"

namespace model {

wxString TransitionParameterFilename::sParameterImageFilename{ "imagefilename" };

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

TransitionParameterFilename::TransitionParameterFilename()
    : TransitionParameter()
{
    VAR_DEBUG(*this);
}

TransitionParameterFilename::TransitionParameterFilename(const wxString& fileDescriptor, bool requiresMaskOrAlpha, bool requiresAlpha, const wxString& defaultPath)
    : TransitionParameter()
    , mValue{ }
    , mDefaultPath{ defaultPath }
    , mFileDescriptor{ fileDescriptor }
    , mMaskOrAlpha{ requiresMaskOrAlpha }
    , mAlpha{ requiresAlpha }
{
    VAR_DEBUG(*this);
    ASSERT_IMPLIES(mAlpha, !mMaskOrAlpha);
    ASSERT_IMPLIES(mMaskOrAlpha, !mAlpha);
}

TransitionParameterFilename::TransitionParameterFilename(const TransitionParameterFilename& other)
    : TransitionParameter(other)
    , mValue{ other.mValue }
    , mDefaultPath{ other.mDefaultPath }
    , mFileDescriptor{ other.mFileDescriptor }
    , mMaskOrAlpha{ other.mMaskOrAlpha }
    , mAlpha{ other.mAlpha }
{
    VAR_DEBUG(*this);
}

TransitionParameterFilename* TransitionParameterFilename::clone() const
{
    return new TransitionParameterFilename(static_cast<const TransitionParameterFilename&>(*this));
}

TransitionParameterFilename::~TransitionParameterFilename()
{
    VAR_DEBUG(this);
}

//////////////////////////////////////////////////////////////////////////
// TRANSITIONPARAMETER
//////////////////////////////////////////////////////////////////////////

void TransitionParameterFilename::copyValue(TransitionParameterPtr other)
{
    boost::shared_ptr<TransitionParameterFilename> typed{ boost::dynamic_pointer_cast<TransitionParameterFilename>(other) };
    if (typed)
    {
        setValue(typed->getValue());
    }
}

wxWindow* TransitionParameterFilename::makeWidget(wxWindow *parent)
{
    ASSERT_ZERO(mPanel);
    ASSERT_ZERO(mFile);
    ASSERT_ZERO(mFileButton);

    mPanel = new wxPanel(parent);
    mPanel->SetSizer(new wxBoxSizer(wxHORIZONTAL));
    mFile = new wxTextCtrl(mPanel, wxID_ANY, mValue.GetLongPath(), wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
    mFile->SetValue(mValue.GetLongPath());
    mFile->SetToolTip(getToolTip());
    mFileButton = new wxButton(mPanel, wxID_ANY, _("Select"));
    mFileButton->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &TransitionParameterFilename::onFileButtonPressed, this);
    mFileButton->SetToolTip(getToolTip());
    mPanel->GetSizer()->Add(mFile, wxSizerFlags(1).Expand());
    mPanel->GetSizer()->Add(mFileButton, wxSizerFlags(0));
    mPanel->SetToolTip(getToolTip());

    return mPanel;
}
void TransitionParameterFilename::destroyWidget()
{
    ASSERT_NONZERO(mPanel);
    ASSERT_NONZERO(mFile);
    ASSERT_NONZERO(mFileButton);

    mFileButton->Unbind(wxEVT_COMMAND_BUTTON_CLICKED, &TransitionParameterFilename::onFileButtonPressed, this);

    mPanel->Destroy();
    mPanel = nullptr;
    mFile = nullptr;
    mFileButton = nullptr;
}
//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

wxFileName TransitionParameterFilename::getValue() const
{
    return mValue;
}

void TransitionParameterFilename::setValue(wxFileName value)
{
    if (mValue != value)
    {
        VAR_INFO(value);
        mValue = value;
        if (mFile != nullptr)
        {
            mFile->SetValue(mValue.GetLongPath());
        }
        signalUpdate();
    }
}


//////////////////////////////////////////////////////////////////////////
// GUI EVENTS
//////////////////////////////////////////////////////////////////////////

void TransitionParameterFilename::onFileButtonPressed(wxCommandEvent& event)
{
    CatchExceptions([this]
    {
        ASSERT_NONZERO(mPanel);
        wxString selected{ gui::Dialog::get().getFile(getDescription(), mDefaultPath, mFileDescriptor, mPanel) };
        if (!selected.IsEmpty())
        {
            wxFileName value{ util::path::toFileName(selected) };
            value.MakeAbsolute();
            setValue(value);
        }
    });
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, const TransitionParameterFilename& obj)
{
    os << obj.mValue;
    return os;
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void TransitionParameterFilename::serialize(Archive & ar, const unsigned int version)
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
template void TransitionParameterFilename::serialize<boost::archive::xml_oarchive>(boost::archive::xml_oarchive& ar, const unsigned int archiveVersion);
template void TransitionParameterFilename::serialize<boost::archive::xml_iarchive>(boost::archive::xml_iarchive& ar, const unsigned int archiveVersion);

} //namespace

BOOST_CLASS_EXPORT_IMPLEMENT(model::TransitionParameterFilename)
