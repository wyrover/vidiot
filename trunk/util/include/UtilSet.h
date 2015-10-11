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

#pragma once

#include "UtilLog.h"
#include "UtilLogStl.h"

template<class ELEMENT>
class UtilSet
{
public:
    typedef std::set<ELEMENT> ELEMENTS;

    /// Create a new UtilSet helper object
    /// \param set set which is operated upon
    UtilSet(ELEMENTS& set)
        :   mSet(set)
    {
    }

    /// Add elements to the set
    /// \param added elements to be added
    void addElements(const ELEMENTS& added)
    {
        mSet.insert(added.begin(),added.end());
        VAR_DEBUG(added)(mSet);
    }

    /// Add one element to the set
    /// \param add element to be added
    void addElement(const ELEMENT& add)
    {
        mSet.insert(add);
        VAR_DEBUG(add)(mSet);
    }

    /// \param removed elements to be removed
    void removeElements(const ELEMENTS& removed)
    {
        mSet.erase(removed.begin(),removed.end());
        VAR_DEBUG(removed)(mSet);
    }

    /// Add elements from the list to the set
    /// \param set the set of elements to be added
    void addElements(const std::vector<ELEMENT>& list)
    {
        mSet.insert(list.begin(),list.end());
    }

private:

    ELEMENTS& mSet;

};
