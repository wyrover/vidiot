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

#include "UtilSerializeWxwidgets.h"

namespace boost { namespace serialization {
template<class Archive>
void save(Archive & ar, const wxString & string, const unsigned int version)
{
    std::string s = string.ToStdString();
    ar & s;
}

template<class Archive>
void load(Archive & ar, wxString & string, const unsigned int version)
{
    std::string s;
    ar & s;
    wxString input(wxSafeConvertMB2WX(s.c_str()));
    string = input;
}

template void save<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const wxString & string, const unsigned int version);
template void load<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar,       wxString & string, const unsigned int version);

template<class Archive>
void save(Archive & ar, const wxFileName& filename, const unsigned int version)
{
    ar & filename.GetLongPath();
}

template<class Archive>
void load(Archive & ar, wxFileName& filename, const unsigned int version)
{
    wxString longpath;
    ar & longpath;
    filename.Assign(longpath);
}

template void save<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const wxFileName& filename, const unsigned int version);
template void load<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar,       wxFileName& filename, const unsigned int version);

template<class Archive>
void serialize(Archive & ar, wxPoint & r, const unsigned int version)
{
    ar & r.x;
    ar & r.y;
}

template void serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, wxPoint & r, const unsigned int version);
template void serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, wxPoint & r, const unsigned int version);

template<class Archive>
void serialize(Archive & ar, wxSize & r, const unsigned int version)
{
    ar & r.x;
    ar & r.y;
}

template void serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, wxSize & r, const unsigned int version);
template void serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, wxSize & r, const unsigned int version);

template<class Archive>
void serialize(Archive & ar, wxRect & r, const unsigned int version)
{
    ar & r.x;
    ar & r.y;
    ar & r.width;
    ar & r.height;
}

template void serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, wxRect & r, const unsigned int version);
template void serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, wxRect & r, const unsigned int version);

template<class Archive>
void save(Archive & ar, const wxRegion & region, const unsigned int version)
{
    int n = 0;
    wxRegionIterator it(region);
    while (it)
    {
        n++;
        it++;
    }
    ar & n;
    it.Reset();
    while (it)
    {
        wxRect rect = it.GetRect(); // Needed for GCC
        ar & rect;
        it++;
    }
}

template<class Archive>
void load(Archive & ar, wxRegion & region, const unsigned int version)
{
    int n = 0;
    ar & n;
    wxRect r;
    while (n > 0)
    {
        ar & r;
        region.Union(r);
        n--;
    }
}

template void save<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const wxRegion & region, const unsigned int version);
template void load<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar,       wxRegion & region, const unsigned int version);
}} // namespace boost::serialization