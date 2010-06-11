#ifndef UTIL_SERIALIZE_WXWIDGETS_H
#define UTIL_SERIALIZE_WXWIDGETS_H

#include <wx/string.h>
#include <boost/serialization/split_free.hpp>

BOOST_SERIALIZATION_SPLIT_FREE(wxString)

namespace boost { namespace serialization {

template<class Archive>
void save(Archive & ar, const wxString & string, const unsigned int version)
{
    std::string s = string.mb_str();
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

}} // namespace boost::serialization

#endif //UTIL_SERIALIZE_WXWIDGETS_H

