#ifndef UTIL_PATH_H
#define UTIL_PATH_H

namespace util { namespace path {

/// Expand path such that ~1 etc. on windows is expanded.
/// return normalized path.
/// \see wxFileName::Normalize for details.
wxFileName normalize( wxFileName filename );

/// Convert a wxFileName to a file part only.
/// * For volumes, this returns the volume letter and :, without trailing slash
/// * For folders, this returns the last folder part, without trailing slash.
/// * For files, this returns filename.extension
// \return name to be used (in project view) for identifying the file.
wxString toName( wxFileName filename );

/// Convert a wxFileName to a full path. This ends without a trailing slash.
wxString toPath( wxFileName filename );

}} // namespace

#endif //UTIL_PATH_H