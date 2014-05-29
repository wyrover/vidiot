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

#include "UtilSerializeWxwidgets.h"

namespace boost { namespace serialization {

const std::string sString("string");

template<class Archive>
void save(Archive & ar, const wxString & string, const unsigned int version)
{
    try
    {
        std::string s = string.ToStdString();
        ar & boost::serialization::make_nvp(sString.c_str(),s);
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
        ar & boost::serialization::make_nvp(sString.c_str(),s);
        wxString input(wxSafeConvertMB2WX(s.c_str()));
        string = input;
    }
    catch (boost::archive::archive_exception& e) { VAR_ERROR(e.what());                         throw; }
    catch (boost::exception &e)                  { VAR_ERROR(boost::diagnostic_information(e)); throw; }
    catch (std::exception& e)                    { VAR_ERROR(e.what());                         throw; }
    catch (...)                                  { LOG_ERROR;                                   throw; }
}

template void save<boost::archive::xml_oarchive>(boost::archive::xml_oarchive& ar, const wxString & string, const unsigned int version);
template void load<boost::archive::xml_iarchive>(boost::archive::xml_iarchive& ar,       wxString & string, const unsigned int version);

const std::string sFileName("filename");

template<class Archive>
void save(Archive & ar, const wxFileName& filename, const unsigned int version)
{
    try
    {
        wxString path = filename.GetLongPath();
        ar & boost::serialization::make_nvp(sFileName.c_str(),path);
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
        ar & boost::serialization::make_nvp(sFileName.c_str(),longpath);
        filename.Assign(longpath);
    }
    catch (boost::archive::archive_exception& e) { VAR_ERROR(e.what());                         throw; }
    catch (boost::exception &e)                  { VAR_ERROR(boost::diagnostic_information(e)); throw; }
    catch (std::exception& e)                    { VAR_ERROR(e.what());                         throw; }
    catch (...)                                  { LOG_ERROR;                                   throw; }
}

template void save<boost::archive::xml_oarchive>(boost::archive::xml_oarchive& ar, const wxFileName& filename, const unsigned int version);
template void load<boost::archive::xml_iarchive>(boost::archive::xml_iarchive& ar,       wxFileName& filename, const unsigned int version);

template<class Archive>
void serialize(Archive & ar, wxPoint & r, const unsigned int version)
{
    try
    {
        ar & boost::serialization::make_nvp("x",r.x);
        ar & boost::serialization::make_nvp("y",r.y);
    }
    catch (boost::archive::archive_exception& e) { VAR_ERROR(e.what());                         throw; }
    catch (boost::exception &e)                  { VAR_ERROR(boost::diagnostic_information(e)); throw; }
    catch (std::exception& e)                    { VAR_ERROR(e.what());                         throw; }
    catch (...)                                  { LOG_ERROR;                                   throw; }
}

template void serialize<boost::archive::xml_oarchive>(boost::archive::xml_oarchive& ar, wxPoint & r, const unsigned int version);
template void serialize<boost::archive::xml_iarchive>(boost::archive::xml_iarchive& ar, wxPoint & r, const unsigned int version);

template<class Archive>
void serialize(Archive & ar, wxSize & r, const unsigned int version)
{
    try
    {
        ar & boost::serialization::make_nvp("x",r.x);
        ar & boost::serialization::make_nvp("y",r.y);
    }
    catch (boost::archive::archive_exception& e) { VAR_ERROR(e.what());                         throw; }
    catch (boost::exception &e)                  { VAR_ERROR(boost::diagnostic_information(e)); throw; }
    catch (std::exception& e)                    { VAR_ERROR(e.what());                         throw; }
    catch (...)                                  { LOG_ERROR;                                   throw; }
}

template void serialize<boost::archive::xml_oarchive>(boost::archive::xml_oarchive& ar, wxSize & r, const unsigned int version);
template void serialize<boost::archive::xml_iarchive>(boost::archive::xml_iarchive& ar, wxSize & r, const unsigned int version);

template<class Archive>
void serialize(Archive & ar, wxRect & r, const unsigned int version)
{
    try
    {
        ar & boost::serialization::make_nvp("x",r.x);
        ar & boost::serialization::make_nvp("y",r.y);
        ar & boost::serialization::make_nvp("width",r.width);
        ar & boost::serialization::make_nvp("height",r.height);
    }
    catch (boost::archive::archive_exception& e) { VAR_ERROR(e.what());                         throw; }
    catch (boost::exception &e)                  { VAR_ERROR(boost::diagnostic_information(e)); throw; }
    catch (std::exception& e)                    { VAR_ERROR(e.what());                         throw; }
    catch (...)                                  { LOG_ERROR;                                   throw; }
}

template void serialize<boost::archive::xml_oarchive>(boost::archive::xml_oarchive& ar, wxRect & r, const unsigned int version);
template void serialize<boost::archive::xml_iarchive>(boost::archive::xml_iarchive& ar, wxRect & r, const unsigned int version);

const std::string sNumberOfRegions("numberOfRegions");
const std::string sRect("rect");

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
        ar & boost::serialization::make_nvp(sNumberOfRegions.c_str(),n);
        it.Reset();
        while (it)
        {
            wxRect rect = it.GetRect(); // Needed for GCC
            ar & boost::serialization::make_nvp(sRect.c_str(),rect);
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
        ar & boost::serialization::make_nvp(sNumberOfRegions.c_str(),n);
        wxRect r;
        while (n > 0)
        {
            ar & boost::serialization::make_nvp(sRect.c_str(),r);
            region.Union(r);
            n--;
        }
    }
    catch (boost::archive::archive_exception& e) { VAR_ERROR(e.what());                         throw; }
    catch (boost::exception &e)                  { VAR_ERROR(boost::diagnostic_information(e)); throw; }
    catch (std::exception& e)                    { VAR_ERROR(e.what());                         throw; }
    catch (...)                                  { LOG_ERROR;                                   throw; }
}

template void save<boost::archive::xml_oarchive>(boost::archive::xml_oarchive& ar, const wxRegion & region, const unsigned int version);
template void load<boost::archive::xml_iarchive>(boost::archive::xml_iarchive& ar,       wxRegion & region, const unsigned int version);

}} // namespace boost::serialization
