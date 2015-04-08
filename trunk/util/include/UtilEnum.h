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

// USAGE:
//
// In .h file:
// DECLAREENUM(<nameofnewenum>, <enumvalue1>, <enumvalue2>, ...);
//
// In .cpp file:
// IMPLEMENTENUM(<nameofnewenum>);
//
// Then the enum values can be used as regular enum values.
//
// operator<<is defined for the enum (for logging)
//
// Convert enum value to string:
// std::string <nameofnewenum>_toString( <nameofnewenum> value )
//
// Convert string representation to enum value:
// <nameofnewenum>_fromString( std::string value )

// Twice the same implementation, just because of the different
// handling of variadic arguments....

#include "UtilMap.h"

#ifdef _MSC_VER

#define DECLAREENUM(ENUMNAME,VALUE1,...) \
    enum ENUMNAME { VALUE1 = 0, __VA_ARGS__ , ENUMNAME ## _MAX }; \
class ENUMNAME ## Converter \
{ \
public: \
    ENUMNAME ## Converter() \
    { \
    std::string names = #VALUE1 "," #__VA_ARGS__ ; \
    ENUMNAME values[] = { VALUE1, __VA_ARGS__, ENUMNAME ## _MAX }; \
    boost::char_separator<char> sep(","); \
    boost::tokenizer<boost::char_separator<char> > tokens(names, sep); \
    boost::tokenizer<boost::char_separator<char> >::iterator it = tokens.begin(); \
    int c = 0; \
    while (it != tokens.end()) \
        { \
        std::string s = *it; \
        s.erase(std::remove_if(s.begin(), s.end(), ::isspace), s.end()); \
        mMap[values[c]] = s; \
        ++it; \
        ++c; \
        } \
    }; \
    virtual ~ENUMNAME ## Converter() {}; \
    static ENUMNAME ## Converter sConverter; \
    typedef std::map<ENUMNAME,wxString> ENUMNAME ## Map; \
    static ENUMNAME ## Map mapToHumanReadibleString; \
    wxString toString( const ENUMNAME& value ) const { return mMap.find(value)->second; }; \
    ENUMNAME fromString( const wxString& value ) const { return UtilMap<ENUMNAME,wxString>(mMap).reverseLookup(value, ENUMNAME ## _MAX); }; \
    static ENUMNAME readConfigValue(const wxString& path); \
private: \
    std::map<ENUMNAME,wxString> mMap; \
}; \
std::ostream& operator<<(std::ostream& os, const ENUMNAME& obj); \
wxString ENUMNAME ## _toString( const ENUMNAME& value ); \
ENUMNAME ENUMNAME ## _fromString( const wxString& value ); \
ENUMNAME Enum_fromConfig(const wxString& value, const ENUMNAME& unused);

#else // _MSC_VER

// Avoid 'redefined' warning in GCC
#undef DECLAREENUM
#undef IMPLEMENTENUM

#define DECLAREENUM(ENUMNAME,VALUE1,OTHERVALUES...) \
    enum ENUMNAME : int { VALUE1 = 0, OTHERVALUES , ENUMNAME ## _MAX }; \
class ENUMNAME ## Converter \
{ \
public: \
    ENUMNAME ## Converter() \
    { \
    std::string names = #VALUE1 "," #OTHERVALUES ; \
    ENUMNAME values[] = { VALUE1, OTHERVALUES, ENUMNAME ## _MAX }; \
        boost::char_separator<char> sep(","); \
        boost::tokenizer<boost::char_separator<char> > tokens(names, sep); \
        boost::tokenizer<boost::char_separator<char> >::iterator it = tokens.begin(); \
        int c = 0; \
        while (it != tokens.end()) \
        { \
            std::string s = *it; \
            s.erase(std::remove_if(s.begin(), s.end(), ::isspace), s.end()); \
            mMap[values[c]] = s; \
            ++it; \
            ++c; \
        } \
    }; \
    virtual ~ENUMNAME ## Converter() {}; \
    static ENUMNAME ## Converter sConverter; \
    typedef std::map<ENUMNAME,wxString> ENUMNAME ## Map; \
    static ENUMNAME ## Map mapToHumanReadibleString; \
    wxString toString( const ENUMNAME& value ) const { return mMap.find(value)->second; }; \
    ENUMNAME fromString( const wxString& value ) const { return UtilMap<ENUMNAME,wxString>(mMap).reverseLookup(value, ENUMNAME ## _MAX); }; \
    static ENUMNAME readConfigValue(const wxString& path); \
private: \
    std::map<ENUMNAME,wxString> mMap; \
}; \
std::ostream& operator<<(std::ostream& os, const ENUMNAME& obj); \
wxString ENUMNAME ## _toString( const ENUMNAME& value ); \
ENUMNAME ENUMNAME ## _fromString( const wxString& value ); \
ENUMNAME Enum_fromConfig(const wxString& value, const ENUMNAME& unused);

#endif

// First line creates the instantation of the conversion object
// Second line uses this object to provide the stream operator<<
#define IMPLEMENTENUM(ENUMNAME) \
ENUMNAME ## Converter ENUMNAME ## Converter::sConverter; \
std::ostream& operator<<(std::ostream& os, const ENUMNAME& obj) { os << ENUMNAME ## _toString(obj); return os; }; \
wxString ENUMNAME ## _toString( const ENUMNAME& value ) { return ENUMNAME ## Converter::sConverter.toString(value); }; \
ENUMNAME ENUMNAME ## _fromString( const wxString& value ) { return ENUMNAME ## Converter::sConverter.fromString(value); }; \
ENUMNAME Enum_fromConfig(const wxString& value, const ENUMNAME& unused) { return ENUMNAME ## _fromString(wxString(value.mb_str())); }; \
ENUMNAME ENUMNAME ## Converter::readConfigValue(const wxString& path) { return ENUMNAME ## _fromString(wxString(Config::ReadString(path).mb_str())); };
