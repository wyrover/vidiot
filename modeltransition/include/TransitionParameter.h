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

namespace model {

DECLARE_EVENT(EVENT_TRANSITION_PARAMETER_CHANGED, EventTransitionParameterChanged, wxString);

class TransitionParameter
    :   public wxEvtHandler // MUST BE FIRST INHERITED CLASS FOR WXWIDGETS EVENTS TO BE RECEIVED.
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    /// Constructor for recovery from disk and for creating a new parameter.
    TransitionParameter() = default;

    /// Used for making deep copies (clones)
    virtual TransitionParameter* clone() const = 0;
    void onCloned() {};

    virtual ~TransitionParameter() = default;

    //////////////////////////////////////////////////////////////////////////
    // INTERFACE
    //////////////////////////////////////////////////////////////////////////

    virtual void copyValue(TransitionParameterPtr other) = 0;

    virtual wxWindow* makeWidget(wxWindow *parent) = 0;
    virtual void destroyWidget() = 0;

    wxString getName() const;
    void setName(const wxString& name);

    wxString getDescription() const;
    void setDescription(const wxString& description);

    wxString getToolTip() const;
    void setToolTip(const wxString& tooltip);

    wxBitmap getBitmap();
    void setBitmap(const wxString& name);
    
    void setOnChanged(std::function<void(const wxString&)> onChange);

protected:

    //////////////////////////////////////////////////////////////////////////
    // COPY CONSTRUCTOR
    //////////////////////////////////////////////////////////////////////////

    /// Copy constructor. Use make_cloned for making deep copies of objects.
    /// \see make_cloned
    TransitionParameter(const TransitionParameter& other);

    //////////////////////////////////////////////////////////////////////////
    // TO BE CALLED WHEN THE DATA CHANGES
    //////////////////////////////////////////////////////////////////////////

    /// This ensures events are generated.
    void signalUpdate();

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    wxString mName; // Never serialize this
    wxString mDescription; // Never serialize this
    wxString mToolTip; // Never serialize this
    wxBitmap mBitmap; // Never serialize this
    std::function<void(const wxString&)> mOnChange = nullptr;

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<<(std::ostream& os, const TransitionParameter& obj);

    //////////////////////////////////////////////////////////////////////////
    // SERIALIZATION
    //////////////////////////////////////////////////////////////////////////

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);
};

} // namespace

BOOST_CLASS_VERSION(model::TransitionParameter, 1)
BOOST_CLASS_EXPORT_KEY(model::TransitionParameter)
