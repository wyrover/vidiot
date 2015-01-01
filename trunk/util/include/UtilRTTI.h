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

#ifndef UTIL_RTTI_H
#define UTIL_RTTI_H

/// Needed for checking object types in class hierarchies
class IRTTI
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    IRTTI() {};
    virtual ~IRTTI() {}; /// One virtual method required for RTTI

    //////////////////////////////////////////////////////////////////////////
    // IRTTI
    //////////////////////////////////////////////////////////////////////////
    
    template <typename Derived>
    bool isA() 
    {
        Derived* derived = dynamic_cast<Derived*>(this);
        return (derived != 0);
    }
};

#endif
