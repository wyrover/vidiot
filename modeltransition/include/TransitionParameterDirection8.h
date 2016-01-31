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

DECLAREENUM(Direction8, \
    Direction8TopLeftToBottomRight, \
    Direction8TopToBottom, \
    Direction8TopRightToBottomLeft, \
    Direction8RightToLeft, \
    Direction8BottomRightToTopLeft, \
    Direction8BottomToTop, \
    Direction8BottomLeftToTopRight, \
    Direction8LeftToRight);

typedef EnumSelector<Direction8> Direction8Selector;

class TransitionParameterDirection8
    : public TransitionParameter
{
public:

    // Generic settings used by multiple transitions.
    // Defining them here ensures that these parameters are maintained when
    // changing the transition type.
    static wxString sParameterDirection8;

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    /// Constructor for recovery from disk.
    TransitionParameterDirection8();

    /// Constructor for creating a new parameter.
    explicit TransitionParameterDirection8(const Direction8& direction);

    /// Used for making deep copies (clones)
    virtual TransitionParameterDirection8* clone() const override;

    virtual ~TransitionParameterDirection8();

    //////////////////////////////////////////////////////////////////////////
    // TRANSITIONPARAMETER
    //////////////////////////////////////////////////////////////////////////

    void copyValue(TransitionParameterPtr other) override;

    wxWindow* makeWidget(wxWindow *parent) override;

    void destroyWidget() override;

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    Direction8 getValue() const;
    void setValue(Direction8 value);

protected:

    //////////////////////////////////////////////////////////////////////////
    // COPY CONSTRUCTOR
    //////////////////////////////////////////////////////////////////////////

    /// Copy constructor. Use make_cloned for making deep copies of objects.
    /// \see make_cloned
    TransitionParameterDirection8(const TransitionParameterDirection8& other);

private:

    //////////////////////////////////////////////////////////////////////////
    // GUI EVENTS
    //////////////////////////////////////////////////////////////////////////

    void onDirection(wxCommandEvent& event);

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    Direction8Selector* mControl = nullptr;
    Direction8 mValue = Direction8_MAX;

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<<(std::ostream& os, const TransitionParameterDirection8& obj);

    //////////////////////////////////////////////////////////////////////////
    // SERIALIZATION
    //////////////////////////////////////////////////////////////////////////

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);
};
} // namespace

BOOST_CLASS_VERSION(model::TransitionParameterDirection8, 1)
BOOST_CLASS_EXPORT_KEY(model::TransitionParameterDirection8)
