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

#pragma once

#include "TransitionParameter.h"

namespace model {

class TransitionParameterFilename
    : public TransitionParameter
{
public:

    // Generic settings used by multiple transitions.
    // Defining them here ensures that these parameters are maintained when
    // changing the transition type.
    static wxString sParameterImageFilename;

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    /// Constructor for recovery from disk.
    TransitionParameterFilename();

    /// Constructor for creating a new parameter.
    /// \param fileDescriptor descriptor for selecting file types in select file dialog
    /// \param defaultPath if nonempty indicates the default path for searching images
    /// \param requiresMaskOrAlpha if true, the image MUST have either a mask OR an alpha channel
    /// \param requiresAlpha if true, the image MUST have an alpha channel
    /// \pre !requiresMaskOrAlpha || !requiresAlpha
    explicit TransitionParameterFilename(const wxString& fileDescriptor, bool requiresMaskOrAlpha, bool requiresAlpha, const wxString& defaultPath = wxEmptyString);

    /// Used for making deep copies (clones)
    virtual TransitionParameterFilename* clone() const override;

    virtual ~TransitionParameterFilename();

    //////////////////////////////////////////////////////////////////////////
    // TRANSITIONPARAMETER
    //////////////////////////////////////////////////////////////////////////

    void copyValue(TransitionParameterPtr other) override;

    wxWindow* makeWidget(wxWindow *parent) override;
    void destroyWidget() override;

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    wxFileName getValue() const;
    void setValue(wxFileName value);

protected:

    //////////////////////////////////////////////////////////////////////////
    // COPY CONSTRUCTOR
    //////////////////////////////////////////////////////////////////////////

    /// Copy constructor. Use make_cloned for making deep copies of objects.
    /// \see make_cloned
    TransitionParameterFilename(const TransitionParameterFilename& other);

private:

    //////////////////////////////////////////////////////////////////////////
    // GUI EVENTS
    //////////////////////////////////////////////////////////////////////////

    void onFileButtonPressed(wxCommandEvent& event);

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    wxPanel* mPanel = nullptr;
    wxTextCtrl* mFile = nullptr;
    wxButton* mFileButton = nullptr;
    wxFileName mValue;
    wxString mDefaultPath;
    wxString mFileDescriptor;
    bool mMaskOrAlpha = false; ///< If true, the image MUST have a mask OR alpha data
    bool mAlpha = false; ///< If true, the image MUST have alpha data

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<<(std::ostream& os, const TransitionParameterFilename& obj);

    //////////////////////////////////////////////////////////////////////////
    // SERIALIZATION
    //////////////////////////////////////////////////////////////////////////

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);
};
} // namespace

BOOST_CLASS_VERSION(model::TransitionParameterFilename, 1)
BOOST_CLASS_EXPORT_KEY(model::TransitionParameterFilename)
