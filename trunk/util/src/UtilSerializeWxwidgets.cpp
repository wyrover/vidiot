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
    try
    {
        std::string s = string.ToStdString();
        ar & s;
    }
    catch (boost::archive::archive_exception& e) { VAR_ERROR(e.what());                         throw; }
    catch (boost::exception &e)                  { VAR_ERROR(boost::diagnostic_information(e)); throw; }
    catch (std::exception& e)                    { VAR_ERROR(e.what());                         throw; }
    catch (...)                                  { LOG_ERROR;                                   throw; }
}

template<class Archive>
void load(Archive & ar, wxString & string, const unsigned int version)
{
    try
    {
        std::string s;
        ar & s;
        wxString input(wxSafeConvertMB2WX(s.c_str()));
        string = input;
    }
    catch (boost::archive::archive_exception& e) { VAR_ERROR(e.what());                         throw; }
    catch (boost::exception &e)                  { VAR_ERROR(boost::diagnostic_information(e)); throw; }
    catch (std::exception& e)                    { VAR_ERROR(e.what());                         throw; }
    catch (...)                                  { LOG_ERROR;                                   throw; }
}

template void save<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const wxString & string, const unsigned int version);
template void load<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar,       wxString & string, const unsigned int version);

template<class Archive>
void save(Archive & ar, const wxFileName& filename, const unsigned int version)
{
    try
    {
        ar & filename.GetLongPath();
    }
    catch (boost::archive::archive_exception& e) { VAR_ERROR(e.what());                         throw; }
    catch (boost::exception &e)                  { VAR_ERROR(boost::diagnostic_information(e)); throw; }
    catch (std::exception& e)                    { VAR_ERROR(e.what());                         throw; }
    catch (...)                                  { LOG_ERROR;                                   throw; }
}

template<class Archive>
void load(Archive & ar, wxFileName& filename, const unsigned int version)
{
    try
    {
        wxString longpath;
        ar & longpath;
        filename.Assign(longpath);
    }
    catch (boost::archive::archive_exception& e) { VAR_ERROR(e.what());                         throw; }
    catch (boost::exception &e)                  { VAR_ERROR(boost::diagnostic_information(e)); throw; }
    catch (std::exception& e)                    { VAR_ERROR(e.what());                         throw; }
    catch (...)                                  { LOG_ERROR;                                   throw; }
}

template void save<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const wxFileName& filename, const unsigned int version);
template void load<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar,       wxFileName& filename, const unsigned int version);

template<class Archive>
void serialize(Archive & ar, wxPoint & r, const unsigned int version)
{
    try
    {
        ar & r.x;
        ar & r.y;
    }
    catch (boost::archive::archive_exception& e) { VAR_ERROR(e.what());                         throw; }
    catch (boost::exception &e)                  { VAR_ERROR(boost::diagnostic_information(e)); throw; }
    catch (std::exception& e)                    { VAR_ERROR(e.what());                         throw; }
    catch (...)                                  { LOG_ERROR;                                   throw; }
}

template void serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, wxPoint & r, const unsigned int version);
template void serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, wxPoint & r, const unsigned int version);

template<class Archive>
void serialize(Archive & ar, wxSize & r, const unsigned int version)
{
    try
    {
        ar & r.x;
        ar & r.y;
    }
    catch (boost::archive::archive_exception& e) { VAR_ERROR(e.what());                         throw; }
    catch (boost::exception &e)                  { VAR_ERROR(boost::diagnostic_information(e)); throw; }
    catch (std::exception& e)                    { VAR_ERROR(e.what());                         throw; }
    catch (...)                                  { LOG_ERROR;                                   throw; }
}

template void serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, wxSize & r, const unsigned int version);
template void serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, wxSize & r, const unsigned int version);

template<class Archive>
void serialize(Archive & ar, wxRect & r, const unsigned int version)
{
    try
    {
        ar & r.x;
        ar & r.y;
        ar & r.width;
        ar & r.height;
    }
    catch (boost::archive::archive_exception& e) { VAR_ERROR(e.what());                         throw; }
    catch (boost::exception &e)                  { VAR_ERROR(boost::diagnostic_information(e)); throw; }
    catch (std::exception& e)                    { VAR_ERROR(e.what());                         throw; }
    catch (...)                                  { LOG_ERROR;                                   throw; }
}

template void serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, wxRect & r, const unsigned int version);
template void serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, wxRect & r, const unsigned int version);

template<class Archive>
void save(Archive & ar, const wxRegion & region, const unsigned int version)
{
    try
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
    catch (boost::archive::archive_exception& e) { VAR_ERROR(e.what());                         throw; }
    catch (boost::exception &e)                  { VAR_ERROR(boost::diagnostic_information(e)); throw; }
    catch (std::exception& e)                    { VAR_ERROR(e.what());                         throw; }
    catch (...)                                  { LOG_ERROR;                                   throw; }
}

template<class Archive>
void load(Archive & ar, wxRegion & region, const unsigned int version)
{
    try
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
    catch (boost::archive::archive_exception& e) { VAR_ERROR(e.what());                         throw; }
    catch (boost::exception &e)                  { VAR_ERROR(boost::diagnostic_information(e)); throw; }
    catch (std::exception& e)                    { VAR_ERROR(e.what());                         throw; }
    catch (...)                                  { LOG_ERROR;                                   throw; }
}

template void save<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const wxRegion & region, const unsigned int version);
template void load<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar,       wxRegion & region, const unsigned int version);
}} // namespace boost::serialization