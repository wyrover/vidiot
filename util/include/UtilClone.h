// Copyright 2013-2016 Eric Raijmakers.
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

#include "UtilRTTI.h"

template <typename T>
boost::shared_ptr<T> make_cloned(const boost::shared_ptr<T>& t)
{
    boost::shared_ptr<T> clone = boost::shared_ptr<T>(t->clone());
    clone->onCloned();
    return clone;
}

template <typename T>
std::vector< boost::shared_ptr<T> > make_cloned(const std::vector< boost::shared_ptr<T> >& elements)
{
    std::vector< boost::shared_ptr<T> > result;
    for ( boost::shared_ptr<T> element : elements )
    {
        result.push_back(make_cloned<T>(element));
    }
    return result;
}

template <typename INDEX, typename T>
std::map< INDEX, boost::shared_ptr<T> > make_cloned(const std::map< INDEX, boost::shared_ptr<T> >& elements)
{
    std::map< INDEX, boost::shared_ptr<T> > result;
    for ( auto index_and_element : elements )
    {
        result[index_and_element.first] = make_cloned<T>(index_and_element.second);
    }
    return result;
}
