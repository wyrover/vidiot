// Copyright 2013-2015 Eric Raijmakers.
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

#ifndef MODEL_TRANSITION_FACTORY_H
#define MODEL_TRANSITION_FACTORY_H

namespace model {

class TransitionFactory
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    TransitionFactory(const std::string& type);

    virtual ~TransitionFactory();

    //////////////////////////////////////////////////////////////////////////
    // TRANSITIONS
    //////////////////////////////////////////////////////////////////////////

    std::vector<model::TransitionPtr> getAllPossibleTransitions() const;
    TransitionPtr getDefault();

protected:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    void add(const TransitionPtr& transition);

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    std::string mType;
    std::vector<model::TransitionPtr> mTransitions;

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<<(std::ostream& os, const TransitionFactory& obj);
};

} // namespace

#endif
