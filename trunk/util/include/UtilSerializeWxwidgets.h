#ifndef UTIL_SERIALIZE_WXWIDGETS_H
#define UTIL_SERIALIZE_WXWIDGETS_H

#include <wx/string.h>
#include <wx/region.h>
#include <boost/serialization/split_free.hpp>

BOOST_SERIALIZATION_SPLIT_FREE(wxString)
BOOST_SERIALIZATION_SPLIT_FREE(wxRegion)

namespace boost { namespace serialization {

template<class Archive> void save(Archive & ar, const wxString & string, const unsigned int version);
template<class Archive> void load(Archive & ar,       wxString & string, const unsigned int version);

template<class Archive> void serialize(Archive & ar, wxRect & r, const unsigned int version);
template<class Archive> void serialize(Archive & ar, wxRect & r, const unsigned int version);

template<class Archive> void save(Archive & ar, const wxRegion & region, const unsigned int version);
template<class Archive> void load(Archive & ar,       wxRegion & region, const unsigned int version);

}} // namespace boost::serialization

#endif //UTIL_SERIALIZE_WXWIDGETS_H

