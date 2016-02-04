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

DECLAREENUM(Direction2, \
    Direction2Horizontal, \
    Direction2Vertical);

typedef EnumSelector<Direction2> Direction2Selector;

class TransitionParameterDirection2
    : public TransitionParameter
{
public:

    // Generic settings used by multiple transitions.
    // Defining them here ensures that these parameters are maintained when
    // changing the transition type.
    static wxString sParameterDirection2;

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    /// Constructor for recovery from disk.
    TransitionParameterDirection2();

    /// Constructor for creating a new parameter.
    explicit TransitionParameterDirection2(const Direction2& direction);

    /// Used for making deep copies (clones)
    virtual TransitionParameterDirection2* clone() const override;

    virtual ~TransitionParameterDirection2();

    //////////////////////////////////////////////////////////////////////////
    // TRANSITIONPARAMETER
    //////////////////////////////////////////////////////////////////////////

    void copyValue(TransitionParameterPtr other) override;

    wxWindow* makeWidget(wxWindow *parent) override;

    void destroyWidget() override;

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    Direction2 getValue() const;
    void setValue(Direction2 value);

protected:

    //////////////////////////////////////////////////////////////////////////
    // COPY CONSTRUCTOR
    //////////////////////////////////////////////////////////////////////////

    /// Copy constructor. Use make_cloned for making deep copies of objects.
    /// \see make_cloned
    TransitionParameterDirection2(const TransitionParameterDirection2& other);

private:

    //////////////////////////////////////////////////////////////////////////
    // GUI EVENTS
    //////////////////////////////////////////////////////////////////////////

    void onDirection(wxCommandEvent& event);

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    Direction2Selector* mControl = nullptr;
    Direction2 mValue = Direction2_MAX;

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<<(std::ostream& os, const TransitionParameterDirection2& obj);

    //////////////////////////////////////////////////////////////////////////
    // SERIALIZATION
    //////////////////////////////////////////////////////////////////////////

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);
};
} // namespace

BOOST_CLASS_VERSION(model::TransitionParameterDirection2, 1)
BOOST_CLASS_EXPORT_KEY(model::TransitionParameterDirection2)
