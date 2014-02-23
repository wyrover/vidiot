// Copyright 2013,2014 Eric Raijmakers.
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

#ifndef UTIL_CLONE_H
#define UTIL_CLONE_H

#include "UtilRTTI.h"

template <typename T>
boost::shared_ptr<T> make_cloned(boost::shared_ptr<T> t)
{
    boost::shared_ptr<T> clone = boost::shared_ptr<T>(t->clone());
    clone->onCloned();
    return clone;
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

#endif
