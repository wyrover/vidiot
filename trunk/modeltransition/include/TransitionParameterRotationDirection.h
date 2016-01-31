// Copyright 2016 Eric Raijmakers.
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

#include "UtilEnum.h"
#include "UtilEnumSelector.h"

namespace model {

DECLAREENUM(RotationDirection, \
    RotationDirectionClockWise, \
    RotationDirectionCounterClockWise);

typedef EnumSelector<RotationDirection> RotationDirectionSelector;

class TransitionParameterRotationDirection
    : public TransitionParameter
{
public:

    // Generic settings used by multiple transitions.
    // Defining them here ensures that these parameters are maintained when
    // changing the transition type.
    static wxString sParameterRotationDirection;

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    /// Constructor for recovery from disk.
    TransitionParameterRotationDirection();

    /// Constructor for creating a new parameter.
    explicit TransitionParameterRotationDirection(const RotationDirection& direction);

    /// Used for making deep copies (clones)
    virtual TransitionParameterRotationDirection* clone() const override;

    virtual ~TransitionParameterRotationDirection();

    //////////////////////////////////////////////////////////////////////////
    // TRANSITIONPARAMETER
    //////////////////////////////////////////////////////////////////////////

    void copyValue(TransitionParameterPtr other) override;

    wxWindow* makeWidget(wxWindow *parent) override;

    void destroyWidget() override;

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    RotationDirection getValue() const;
    void setValue(RotationDirection value);

protected:

    //////////////////////////////////////////////////////////////////////////
    // COPY CONSTRUCTOR
    //////////////////////////////////////////////////////////////////////////

    /// Copy constructor. Use make_cloned for making deep copies of objects.
    /// \see make_cloned
    TransitionParameterRotationDirection(const TransitionParameterRotationDirection& other);

private:

    //////////////////////////////////////////////////////////////////////////
    // GUI EVENTS
    //////////////////////////////////////////////////////////////////////////

    void onRotationDirection(wxCommandEvent& event);

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    RotationDirectionSelector* mControl = nullptr;
    RotationDirection mValue = RotationDirection_MAX;

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<<(std::ostream& os, const TransitionParameterRotationDirection& obj);

    //////////////////////////////////////////////////////////////////////////
    // SERIALIZATION
    //////////////////////////////////////////////////////////////////////////

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);
};
} // namespace

BOOST_CLASS_VERSION(model::TransitionParameterRotationDirection, 1)
BOOST_CLASS_EXPORT_KEY(model::TransitionParameterRotationDirection)
