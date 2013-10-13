// Copyright 2013 Eric Raijmakers.
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

#ifndef UTIL_CLONEABLE_H
#define UTIL_CLONEABLE_H

#include "UtilRTTI.h"

/// Needed for cloning in class hierarchies starting with abstract base classes.
/// Use the method make_cloned to make copies of objects
template <typename MOSTDERIVED>
struct Cloneable
    : public IRTTI
{
    virtual MOSTDERIVED* clone() const
    {
        return new MOSTDERIVED(static_cast<const MOSTDERIVED&>(*this));
    }
    virtual ~Cloneable() {}
};

template <typename T>
boost::shared_ptr<T> make_cloned_ptr(T& t)
{
    return boost::shared_ptr<T>(t.clone());
}

template <typename T>
boost::shared_ptr<T> make_cloned(boost::shared_ptr<T> t)
{
    return boost::shared_ptr<T>(t->clone());
}

template <typename T>
std::list< boost::shared_ptr<T> > make_cloned(std::list< boost::shared_ptr<T> > elements)
{
    std::list< boost::shared_ptr<T> > result;
    for ( boost::shared_ptr<T> element : elements )
    {
        result.push_back(make_cloned<T>(element));
    }
    return result;
}

#endif // UTIL_CLONEABLE_H