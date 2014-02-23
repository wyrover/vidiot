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

#ifndef UTIL_SERIALIZE_WXWIDGETS_H
#define UTIL_SERIALIZE_WXWIDGETS_H

BOOST_SERIALIZATION_SPLIT_FREE(wxString)
BOOST_SERIALIZATION_SPLIT_FREE(wxFileName)
BOOST_SERIALIZATION_SPLIT_FREE(wxRegion)

namespace boost { namespace serialization {
template<class Archive> void save(Archive& ar, const wxString& string, const unsigned int version);
template<class Archive> void load(Archive& ar,       wxString& string, const unsigned int version);

template<class Archive> void save(Archive& ar, const wxFileName& filename, const unsigned int version);
template<class Archive> void load(Archive& ar,       wxFileName& filename, const unsigned int version);

template<class Archive> void serialize(Archive& ar, wxPoint& r, const unsigned int version);
template<class Archive> void serialize(Archive& ar, wxPoint& r, const unsigned int version);

template<class Archive> void serialize(Archive& ar, wxSize& r, const unsigned int version);
template<class Archive> void serialize(Archive& ar, wxSize& r, const unsigned int version);

template<class Archive> void serialize(Archive& ar, wxRect& r, const unsigned int version);
template<class Archive> void serialize(Archive& ar, wxRect& r, const unsigned int version);

template<class Archive> void save(Archive& ar, const wxRegion& region, const unsigned int version);
template<class Archive> void load(Archive& ar,       wxRegion& region, const unsigned int version);
}} // namespace boost::serialization

#endif
