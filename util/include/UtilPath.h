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
/// \return name to be used (in project view) for identifying the file.
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