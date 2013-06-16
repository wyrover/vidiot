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

bool equals( wxFileName f1, wxFileName f2 ); ///< \return true if the two paths indicate the same file/folder on disk
bool equals( wxString f1,   wxFileName f2 ); ///< \return true if the two paths indicate the same file/folder on disk
bool equals( wxFileName f1, wxString f2 );   ///< \return true if the two paths indicate the same file/folder on disk
bool equals( wxString f1,   wxString f2 );   ///< \return true if the two paths indicate the same file/folder on disk

bool isParentOf( wxFileName parent, wxFileName child ); ///< \return true if parent is a parent path (folder) of child. Note: returns false if both point to the same dir.
bool isParentOf( wxString parent,   wxFileName child ); ///< \return true if parent is a parent path (folder) of child. Note: returns false if both point to the same dir.
bool isParentOf( wxFileName parent, wxString child );   ///< \return true if parent is a parent path (folder) of child. Note: returns false if both point to the same dir.
bool isParentOf( wxString parent,   wxString child );   ///< \return true if parent is a parent path (folder) of child. Note: returns false if both point to the same dir.

}} // namespace

#endif //UTIL_PATH_H