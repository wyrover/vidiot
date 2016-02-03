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

DECLAREENUM(Direction4, \
    Direction4TopToBottom, \
    Direction4RightToLeft, \
    Direction4BottomToTop, \
    Direction4LeftToRight);

typedef EnumSelector<Direction4> Direction4Selector;

class TransitionParameterDirection4
    : public TransitionParameter
{
public:

    // Generic settings used by multiple transitions.
    // Defining them here ensures that these parameters are maintained when
    // changing the transition type.
    static wxString sParameterDirection4;

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    /// Constructor for recovery from disk.
    TransitionParameterDirection4();

    /// Constructor for creating a new parameter.
    explicit TransitionParameterDirection4(const Direction4& direction);

    /// Used for making deep copies (clones)
    virtual TransitionParameterDirection4* clone() const override;

    virtual ~TransitionParameterDirection4();

    //////////////////////////////////////////////////////////////////////////
    // TRANSITIONPARAMETER
    //////////////////////////////////////////////////////////////////////////

    void copyValue(TransitionParameterPtr other) override;

    wxWindow* makeWidget(wxWindow *parent) override;

    void destroyWidget() override;

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    Direction4 getValue() const;
    void setValue(Direction4 value);

protected:

    //////////////////////////////////////////////////////////////////////////
    // COPY CONSTRUCTOR
    //////////////////////////////////////////////////////////////////////////

    /// Copy constructor. Use make_cloned for making deep copies of objects.
    /// \see make_cloned
    TransitionParameterDirection4(const TransitionParameterDirection4& other);

private:

    //////////////////////////////////////////////////////////////////////////
    // GUI EVENTS
    //////////////////////////////////////////////////////////////////////////

    void onDirection(wxCommandEvent& event);

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    Direction4Selector* mControl = nullptr;
    Direction4 mValue = Direction4_MAX;

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<<(std::ostream& os, const TransitionParameterDirection4& obj);

    //////////////////////////////////////////////////////////////////////////
    // SERIALIZATION
    //////////////////////////////////////////////////////////////////////////

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);
};
} // namespace

BOOST_CLASS_VERSION(model::TransitionParameterDirection4, 1)
BOOST_CLASS_EXPORT_KEY(model::TransitionParameterDirection4)
