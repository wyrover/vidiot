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

#ifndef UTIL_PATH_H
#define UTIL_PATH_H

namespace util { namespace path {

/// Expand path such that ~1 etc. on windows is expanded.
/// return normalized path.
/// \see wxFileName::Normalize for details.
wxFileName normalize(wxFileName filename );

/// Convert a wxFileName to a file part only.
/// * For volumes, this returns the volume letter and :, without trailing slash
/// * For folders, this returns the last folder part, without trailing slash.
/// * For files, this returns filename.extension
/// \return name to be used (in project view) for identifying the file.
wxString toName(const wxFileName& filename );

/// Convert a wxFileName to a full path. This ends without a trailing slash.
wxString toPath(const wxFileName& filename );

/// Convert a path to a wxFileName.
/// This includes proper handling for files vs. directories.
/// \param path full path to file/folder
/// \return wxFileName representing this file/folder
wxFileName toFileName(const wxString& path);

/// \return modification time of the given file
/// \param filename given file
time_t lastModifiedTime(const wxFileName& filename);

bool equals(const wxFileName& f1, const wxFileName& f2 ); ///< \return true if the two paths indicate the same file/folder on disk
bool equals(const wxString& f1,   const wxFileName& f2 ); ///< \return true if the two paths indicate the same file/folder on disk
bool equals(const wxFileName& f1, const wxString& f2 );   ///< \return true if the two paths indicate the same file/folder on disk
bool equals(const wxString& f1,   const wxString& f2 );   ///< \return true if the two paths indicate the same file/folder on disk

bool isParentOf(const wxFileName& parent, const wxFileName& child ); ///< \return true if parent is a parent path (folder) of child. Note: returns false if both point to the same dir.
bool isParentOf(const wxString& parent,   const wxFileName& child ); ///< \return true if parent is a parent path (folder) of child. Note: returns false if both point to the same dir.
bool isParentOf(const wxFileName& parent, const wxString& child );   ///< \return true if parent is a parent path (folder) of child. Note: returns false if both point to the same dir.
bool isParentOf(const wxString& parent,   const wxString& child );   ///< \return true if parent is a parent path (folder) of child. Note: returns false if both point to the same dir.

/// \return true if the given directory contains one or more subdirectories.
/// \pre directory must be a directory, not a file.
bool hasSubDirectories(wxFileName directory); 

/// \return the full path to a file in the installation directory
/// \param subdirs sub directory of installation directory where file is located (may have multiple levels separated with '/')
/// \param filename name of file to be retrieved. Leave emtpy for dir only.
wxString toFileInInstallationDirectory(wxString subdirs, wxString filename);

}} // namespace

#endif
