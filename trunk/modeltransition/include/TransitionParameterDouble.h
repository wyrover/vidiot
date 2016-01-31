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

class TransitionParameterDouble
    : public TransitionParameter
{
public:

    // Generic settings used by multiple transitions.
    // Defining them here ensures that these parameters are maintained when
    // changing the transition type.
    static wxString sParameterScaling;

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    /// Constructor for recovery from disk.
    TransitionParameterDouble();

    /// Constructor for creating a new parameter.
    explicit TransitionParameterDouble(double value, double min, double max);

    /// Used for making deep copies (clones)
    virtual TransitionParameterDouble* clone() const override;

    virtual ~TransitionParameterDouble();

    //////////////////////////////////////////////////////////////////////////
    // TRANSITIONPARAMETER
    //////////////////////////////////////////////////////////////////////////

    void copyValue(TransitionParameterPtr other) override;

    wxWindow* makeWidget(wxWindow *parent) override;
    void destroyWidget() override;

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    double getValue() const;
    void setValue(double value);

    double getMin() const;
    double getMax() const;

protected:

    //////////////////////////////////////////////////////////////////////////
    // COPY CONSTRUCTOR
    //////////////////////////////////////////////////////////////////////////

    /// Copy constructor. Use make_cloned for making deep copies of objects.
    /// \see make_cloned
    TransitionParameterDouble(const TransitionParameterDouble& other);

private:

    //////////////////////////////////////////////////////////////////////////
    // GUI EVENTS
    //////////////////////////////////////////////////////////////////////////

    void onSlider(wxCommandEvent& event);
    void onSpin(wxSpinDoubleEvent& event);

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    wxPanel* mPanel = nullptr;
    wxSlider* mSlider = nullptr;
    wxSpinCtrlDouble* mSpin = nullptr;
    double mValue = 0;
    double mMin = 0;
    double mMax = 0;

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<<(std::ostream& os, const TransitionParameterDouble& obj);

    //////////////////////////////////////////////////////////////////////////
    // SERIALIZATION
    //////////////////////////////////////////////////////////////////////////

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);
};
} // namespace

BOOST_CLASS_VERSION(model::TransitionParameterDouble, 1)
BOOST_CLASS_EXPORT_KEY(model::TransitionParameterDouble)
