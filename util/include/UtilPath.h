#ifndef UTIL_PATH_H
#define UTIL_PATH_H

namespace util { namespace path {

wxString toName( wxFileName filename );

wxFileName normalize( wxFileName filename );

}} // namespace

#endif //UTIL_PATH_H