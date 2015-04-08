// Copyright 2015 Eric Raijmakers.
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

#ifndef UTIL_MAP_H
#define UTIL_MAP_H

template<class KEY, class VALUE>
class UtilMap
{
public:
    typedef std::map<KEY,VALUE> ELEMENTS;

    /// Create a new UtilSet helper object
    /// \param set set which is operated upon
    UtilMap(const ELEMENTS& map)
        :   mMap(map)
    {
    }

    /// Do a reverse lookup, given an element value.
    /// If the element can be found with multiple keys,
    /// the first found key is returned.
    /// If the element is not in the list of keys, the
    /// default value is returned if given.
    KEY reverseLookup(const VALUE& value, const KEY& defaultKey) const
    {
        for (auto kvp : mMap)
        {
            if (kvp.second == value)
            {
                return kvp.first;
            }
        }
        return defaultKey;
    }

    std::vector<VALUE> values() const
    {
        std::vector<VALUE> result;
        std::transform(begin(mMap), end(mMap), std::back_inserter(result), [](const typename ELEMENTS::value_type& kvp){return kvp.second; });
        return result;
    }

private:

    ELEMENTS mMap;

};

#endif
