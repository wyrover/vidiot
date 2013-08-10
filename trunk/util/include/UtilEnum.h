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
// operator<< is defined for the enum (for logging)
//
// Convert enum value to string:
// std::string <nameofnewenum>_toString( <nameofnewenum> value )
//
// Convert string representation to enum value:
// <nameofnewenum>_fromString( std::string value )

// Twice the same implementation, just because of the different
// handling of variadic arguments....

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
        mMap.insert( boost::bimap<ENUMNAME,std::string>::value_type(values[c], s) ); \
        ++it; \
        ++c; \
        } \
    }; \
    ~ENUMNAME ## Converter() {}; \
    static ENUMNAME ## Converter sConverter; \
    typedef boost::bimap<ENUMNAME,wxString> ENUMNAME ## Map; \
    static ENUMNAME ## Map mapToHumanReadibleString; \
    std::string toString( ENUMNAME value ) const { return mMap.left.find(value)->second; }; \
    ENUMNAME fromString( std::string value ) const \
    {\
        boost::bimap<ENUMNAME,std::string>::right_const_iterator it = mMap.right.find(value); \
        if (it == mMap.right.end()) { return ENUMNAME ## _MAX; } \
        return mMap.right.find(value)->second; \
    }; \
    static ENUMNAME readConfigValue(wxString path); \
private: \
    boost::bimap<ENUMNAME,std::string> mMap; \
}; \
    std::ostream& operator<< (std::ostream& os, const ENUMNAME& obj); \
    std::string ENUMNAME ## _toString( ENUMNAME value ); \
    ENUMNAME ENUMNAME ## _fromString( std::string value ); \
    ENUMNAME Enum_fromConfig(wxString value, ENUMNAME unused);

#else // _MSC_VER

#define DECLAREENUM(ENUMNAME,VALUE1,OTHERVALUES...) \
    enum ENUMNAME { VALUE1 = 0, OTHERVALUES , ENUMNAME ## _MAX }; \
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
            mMap.insert( boost::bimap<ENUMNAME,std::string>::value_type(values[c], s) ); \
            ++it; \
            ++c; \
        } \
    }; \
    ~ENUMNAME ## Converter() {}; \
    static ENUMNAME ## Converter sConverter; \
    std::string toString( ENUMNAME value ) const { return mMap.left.find(value)->second; }; \
    ENUMNAME fromString( std::string value ) const { return mMap.right.find(value)->second; }; \
private: \
    boost::bimap<ENUMNAME,std::string> mMap; \
}; \
std::ostream& operator<< (std::ostream& os, const ENUMNAME& obj); \
std::string ENUMNAME ## _toString( ENUMNAME value ); \
ENUMNAME ENUMNAME ## _fromString( std::string value )

#endif // _MSC_VER

// First line creates the instantation of the conversion object
// Second line uses this object to provide the stream operator<<
#define IMPLEMENTENUM(ENUMNAME) \
ENUMNAME ## Converter ENUMNAME ## Converter::sConverter; \
std::ostream& operator<< (std::ostream& os, const ENUMNAME& obj) { os << ENUMNAME ## _toString(obj); return os; }; \
std::string ENUMNAME ## _toString( ENUMNAME value ) { return ENUMNAME ## Converter::sConverter.toString(value); }; \
ENUMNAME ENUMNAME ## _fromString( std::string value ) { return ENUMNAME ## Converter::sConverter.fromString(value); }; \
ENUMNAME Enum_fromConfig(wxString value, ENUMNAME unused) { return ENUMNAME ## _fromString(std::string(value.mb_str())); }; \
ENUMNAME ENUMNAME ## Converter::readConfigValue(wxString path) { return ENUMNAME ## _fromString(std::string(Config::ReadString(path).mb_str())); };